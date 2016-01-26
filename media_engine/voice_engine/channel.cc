#include "channel.h"
#include "logging.h"
#include "trace.h"
#include "rtp_dump.h"
#include "audio_frame_operations.h"
#include "voe_external_media.h"
#include "utility.h"
#include "output_mixer.h"
#include "engine_rtp_rtcp.h"

#include "modules/audio_data_segment/include/audio_data_separator.h"

#include "modules/rtp_rtcp/source/rtp_utility.h"

namespace webrtc {
namespace voe {

WebRtc_Word32 Channel::SendData(FrameType frameType,
                                WebRtc_UWord8   payloadType,
                                WebRtc_UWord32  timeStamp,
                                const WebRtc_UWord8*  payloadData,
                                WebRtc_UWord16  payloadSize,
                                const RTPFragmentationHeader* fragmentation) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SendData(frameType=%u, payloadType=%u, timeStamp=%u,"
               " payloadSize=%u, fragmentation=0x%x)",
               frameType, payloadType, timeStamp, payloadSize, fragmentation);

  if (_includeAudioLevelIndication) {
    assert(_rtpAudioProc.get() != NULL);
    // Store current audio level in the RTP/RTCP module.
    // The level will be used in combination with voice-activity state
    // (frameType) to add an RTP header extension
    //_rtpRtcpModule->SetAudioLevel(_rtpAudioProc->level_estimator()->RMS());
  }

  // Push data from ACM to RTP/RTCP-module to deliver audio frame for
  // packetization.
  // This call will trigger Transport::SendPacket() from the RTP/RTCP module.
  if (_rtpRtcpModule->SendOutgoingData((FrameType&)frameType,
                                       payloadType,
                                       timeStamp,
                                       // Leaving the time when this frame was
                                       // received from the capture device as
                                       // undefined for voice for now.
                                       -1,
                                       payloadData,
                                       payloadSize,
                                       fragmentation) == -1) {
    _engineStatisticsPtr->SetLastError(
      VE_RTP_RTCP_MODULE_ERROR, kTraceWarning,
      "Channel::SendData() failed to send data to RTP/RTCP module");
    return -1;
  }
  if (_enablePacketCombine && _dataCombinePtr) {
    _dataCombinePtr->IncreaseBufferPacket();
  }
  _lastLocalTimeStamp = timeStamp;
  _lastPayloadType = payloadType;
  return 0;
}

WebRtc_Word32 Channel::InFrameType(WebRtc_Word16 frameType) {
  /*WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId,_channelId),
  "Channel::InFrameType(frameType=%d)", frameType);*/

  CriticalSectionScoped cs(&_callbackCritSect);
  // 1 indicates speech
  _sendFrameType = (frameType == 1) ? 1 : 0;
  return 0;
}

WebRtc_Word32 Channel::OnRxVadDetected(const int vadDecision) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::OnRxVadDetected(vadDecision=%d)", vadDecision);

  /* CriticalSectionScoped cs(&_callbackCritSect);
  if (_rxVadObserverPtr) {
  _rxVadObserverPtr->OnRxVad(_channelId, vadDecision);
  }*/

  return 0;
}

int Channel::SendPacketImpl(int channel, const void *data, int len) {
  // Packet transmission using WebRtc socket transport
  if (!_externalTransport) {
    int n = _transportPtr->SendPacket(channel, data, len);

    if (n < 0) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::SendPacket() RTP transmission using WebRtc"
                   " sockets failed");
      return -1;
    }

    return n;
  }

  // Packet transmission using external transport transport
  {
    CriticalSectionScoped cs(&_callbackCritSect);

    int n = _transportPtr->SendPacket(channel, data, len);

    if (n < 0) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::SendPacket() RTP transmission using external"
                   " transport failed");
      return -1;
    }

    return n;
  }
}

int Channel::SendPacket(int channel, const void *data, int len) {
  channel = VoEChannelId(channel);
  assert(channel == _channelId);

  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SendPacket(channel=%d, len=%d)", channel, len);

  if (_transportPtr == NULL) {
    WEBRTC_TRACE(kTraceError, kTraceVoice, VoEId(_instanceId, _channelId),
                 "Channel::SendPacket() failed to send RTP packet due to"
                 " invalid transport object");
    return -1;
  }

  // Insert extra RTP packet using if user has called the InsertExtraRTPPacket
  // API
  if (_insertExtraRTPPacket) {
    WebRtc_UWord8* rtpHdr = (WebRtc_UWord8*)data;
    WebRtc_UWord8 M_PT(0);

    if (_extraMarkerBit) {
      M_PT = 0x80;            // set the M-bit
    }

    M_PT += _extraPayloadType;  // set the payload type
    *(++rtpHdr) = M_PT;     // modify the M|PT-byte within the RTP header
    _insertExtraRTPPacket = false;  // insert one packet only
  }

  WebRtc_UWord8* bufferToSendPtr = (WebRtc_UWord8*)data;
  WebRtc_Word32 bufferLength = len;

  // Dump the RTP packet to a file (if RTP dump is enabled).
  if (_rtpDumpOut.DumpPacket((const WebRtc_UWord8*)data, len) == -1) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "Channel::SendPacket() RTP dump to output file failed");
  }

  // SRTP or External encryption
  if (_encrypting) {
    CriticalSectionScoped cs(&_callbackCritSect);

    if (_encryptionPtr) {
      if (!_encryptionRTPBufferPtr) {
        // Allocate memory for encryption buffer one time only
        _encryptionRTPBufferPtr =
          new WebRtc_UWord8[kVoiceEngineMaxIpPacketSizeBytes];
        memset(_encryptionRTPBufferPtr, 0,
               kVoiceEngineMaxIpPacketSizeBytes);
      }

      // Perform encryption (SRTP or external)
      WebRtc_Word32 encryptedBufferLength = 0;
      _encryptionPtr->encrypt(_channelId,
                              bufferToSendPtr,
                              _encryptionRTPBufferPtr,
                              bufferLength,
                              (int*)&encryptedBufferLength);

      if (encryptedBufferLength <= 0) {
        _engineStatisticsPtr->SetLastError(
          VE_ENCRYPTION_FAILED,
          kTraceError, "Channel::SendPacket() encryption failed");
        return -1;
      }

      // Replace default data buffer with encrypted buffer
      bufferToSendPtr = _encryptionRTPBufferPtr;
      bufferLength = encryptedBufferLength;
    }
  }

  if (_enablePacketCombine && _dataCombinePtr) {
    ModuleRTPUtility::RTPHeaderParser rtpParser((WebRtc_UWord8*)bufferToSendPtr,
        bufferLength);
    WebRtcRTPHeader rtp_header;
    rtpParser.Parse(rtp_header);
    _dataCombinePtr->SendPacket(bufferToSendPtr + rtp_header.header.headerLength,
                                rtp_header.header.payloadType, rtp_header.header.timestamp,
                                rtp_header.header.sequenceNumber,
                                bufferLength - rtp_header.header.headerLength);
  } else {
    return SendPacketImpl(channel, bufferToSendPtr, bufferLength);
  }

  //// Packet transmission using WebRtc socket transport
  //if (!_externalTransport) {
  //  int n = _transportPtr->SendPacket(channel, bufferToSendPtr,
  //                                    bufferLength);

  //  if (n < 0) {
  //    WEBRTC_TRACE(kTraceError, kTraceVoice,
  //                 VoEId(_instanceId, _channelId),
  //                 "Channel::SendPacket() RTP transmission using WebRtc"
  //                 " sockets failed");
  //    return -1;
  //  }

  //  return n;
  //}

  //// Packet transmission using external transport transport
  //{
  //  CriticalSectionScoped cs(&_callbackCritSect);

  //  int n = _transportPtr->SendPacket(channel,
  //                                    bufferToSendPtr,
  //                                    bufferLength);

  //  if (n < 0) {
  //    WEBRTC_TRACE(kTraceError, kTraceVoice,
  //                 VoEId(_instanceId, _channelId),
  //                 "Channel::SendPacket() RTP transmission using external"
  //                 " transport failed");
  //    return -1;
  //  }

  //  return n;
  //}

  return 0;
}

int Channel::SendRTCPPacket(int channel, const void *data, int len) {
  channel = VoEChannelId(channel);
  assert(channel == _channelId);

  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SendRTCPPacket(channel=%d, len=%d)", channel, len);
  return 0;
}

uint32_t Channel::SendCombinedDataCallback(const WebRtc_UWord8* payloadData,
    const WebRtc_UWord16 payloadSize) {
  return SendPacketImpl(_channelId, payloadData, payloadSize);
}

uint32_t Channel::SeparatedRtpCallback(const WebRtc_UWord8* payloadData,
                                       const WebRtc_UWord16 payloadSize) {
  const char dummyIP[] = "127.0.0.1";
  IncomingRTPPacket((WebRtc_Word8*)payloadData, payloadSize, dummyIP, 0);
  return 0;
}

void Channel::EnablePacketCombine(bool enable, uint16_t max_packet) {
  if (0 == _dataCombinePtr) {
    _dataCombinePtr = new AudioDataCombiner(this);

    if (_dataCombinePtr && max_packet)
      _dataCombinePtr->SetMaxBufferedPacket(max_packet);

    //_rtpRtcpModule->GetRTPAudioLevelIndicationStatus()
  }

  _enablePacketCombine = enable;
}

void Channel::EnablePacketSeparate(bool enable) {
  if (0 == _dataSeparatorPtr) {
    _dataSeparatorPtr = new AudioDataSeparator(this);
  }

  _enablePacketSeparate = enable;
}

void Channel::IncomingRTPPacket(const WebRtc_Word8* incomingRtpPacket,
                                const WebRtc_Word32 rtpPacketLength,
                                const char* fromIP,
                                const WebRtc_UWord16 fromPort) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::IncomingRTPPacket(rtpPacketLength=%d,"
               " fromIP=%s, fromPort=%u)",
               rtpPacketLength, fromIP, fromPort);

  // Store playout timestamp for the received RTP packet
  // to be used for upcoming delay estimations
  WebRtc_UWord32 playoutTimestamp(0);

  if (GetPlayoutTimeStamp(playoutTimestamp) == 0) {
    _playoutTimeStampRTP = playoutTimestamp;
  }

  WebRtc_UWord8* rtpBufferPtr = (WebRtc_UWord8*)incomingRtpPacket;
  WebRtc_Word32 rtpBufferLength = rtpPacketLength;

  // SRTP or External decryption
  if (_decrypting) {
    CriticalSectionScoped cs(&_callbackCritSect);

    if (_encryptionPtr) {
      if (!_decryptionRTPBufferPtr) {
        // Allocate memory for decryption buffer one time only
        _decryptionRTPBufferPtr =
          new WebRtc_UWord8[kVoiceEngineMaxIpPacketSizeBytes];
      }

      // Perform decryption (SRTP or external)
      WebRtc_Word32 decryptedBufferLength = 0;
      _encryptionPtr->decrypt(_channelId,
                              rtpBufferPtr,
                              _decryptionRTPBufferPtr,
                              rtpBufferLength,
                              (int*)&decryptedBufferLength);

      if (decryptedBufferLength <= 0) {
        _engineStatisticsPtr->SetLastError(
          VE_DECRYPTION_FAILED, kTraceError,
          "Channel::IncomingRTPPacket() decryption failed");
        return;
      }

      // Replace default data buffer with decrypted buffer
      rtpBufferPtr = _decryptionRTPBufferPtr;
      rtpBufferLength = decryptedBufferLength;
    }
  }

  // Dump the RTP packet to a file (if RTP dump is enabled).
  if (_rtpDumpIn.DumpPacket(rtpBufferPtr,
                            (WebRtc_UWord16)rtpBufferLength) == -1) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "Channel::SendPacket() RTP dump to input file failed");
  }

  // Deliver RTP packet to RTP/RTCP module for parsing
  // The packet will be pushed back to the channel thru the
  // OnReceivedPayloadData callback so we don't push it to the ACM here
  if (_rtpRtcpModule->IncomingPacket((const WebRtc_UWord8*)rtpBufferPtr,
                                     (WebRtc_UWord16)rtpBufferLength) == -1) {
    _engineStatisticsPtr->SetLastError(
      VE_SOCKET_TRANSPORT_MODULE_ERROR, kTraceWarning,
      "Channel::IncomingRTPPacket() RTP packet is invalid");
    return;
  }
}

void Channel::IncomingRTCPPacket(const WebRtc_Word8* incomingRtcpPacket,
                                 const WebRtc_Word32 rtcpPacketLength,
                                 const char* fromIP,
                                 const WebRtc_UWord16 fromPort) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::IncomingRTCPPacket(rtcpPacketLength=%d, fromIP=%s,"
               " fromPort=%u)",
               rtcpPacketLength, fromIP, fromPort);

  // Temporary buffer pointer and size for decryption
  WebRtc_UWord8* rtcpBufferPtr = (WebRtc_UWord8*)incomingRtcpPacket;
  WebRtc_Word32 rtcpBufferLength = rtcpPacketLength;

  // Store playout timestamp for the received RTCP packet
  // which will be read by the GetRemoteRTCPData API
  WebRtc_UWord32 playoutTimestamp(0);

  if (GetPlayoutTimeStamp(playoutTimestamp) == 0) {
    _playoutTimeStampRTCP = playoutTimestamp;
  }

  // SRTP or External decryption
  if (_decrypting) {
    CriticalSectionScoped cs(&_callbackCritSect);

    if (_encryptionPtr) {
      if (!_decryptionRTCPBufferPtr) {
        // Allocate memory for decryption buffer one time only
        _decryptionRTCPBufferPtr =
          new WebRtc_UWord8[kVoiceEngineMaxIpPacketSizeBytes];
      }

      // Perform decryption (SRTP or external).
      WebRtc_Word32 decryptedBufferLength = 0;
      _encryptionPtr->decrypt_rtcp(_channelId,
                                   rtcpBufferPtr,
                                   _decryptionRTCPBufferPtr,
                                   rtcpBufferLength,
                                   (int*)&decryptedBufferLength);

      if (decryptedBufferLength <= 0) {
        _engineStatisticsPtr->SetLastError(
          VE_DECRYPTION_FAILED, kTraceError,
          "Channel::IncomingRTCPPacket() decryption failed");
        return;
      }

      // Replace default data buffer with decrypted buffer
      rtcpBufferPtr = _decryptionRTCPBufferPtr;
      rtcpBufferLength = decryptedBufferLength;
    }
  }

  // Dump the RTCP packet to a file (if RTP dump is enabled).
  if (_rtpDumpIn.DumpPacket(rtcpBufferPtr,
                            (WebRtc_UWord16)rtcpBufferLength) == -1) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "Channel::SendPacket() RTCP dump to input file failed");
  }

  // Deliver RTCP packet to RTP/RTCP module for parsing
  if (_rtpRtcpModule->IncomingPacket((const WebRtc_UWord8*)rtcpBufferPtr,
                                     (WebRtc_UWord16)rtcpBufferLength) == -1) {
    _engineStatisticsPtr->SetLastError(
      VE_SOCKET_TRANSPORT_MODULE_ERROR, kTraceWarning,
      "Channel::IncomingRTPPacket() RTCP packet is invalid");
    return;
  }
}

void Channel::OnIncomingSSRCChanged(const WebRtc_Word32 id,
                                    const WebRtc_UWord32 SSRC) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::OnIncomingSSRCChanged(id=%d, SSRC=%d)",
               id, SSRC);
}

void Channel::OnIncomingCSRCChanged(const WebRtc_Word32 id,
                                    const WebRtc_UWord32 CSRC,
                                    const bool added) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::OnIncomingCSRCChanged(id=%d, CSRC=%d, added=%d)",
               id, CSRC, added);
}

void Channel::OnApplicationDataReceived(const WebRtc_Word32 id,
                                        const WebRtc_UWord8 subType,
                                        const WebRtc_UWord32 name,
                                        const WebRtc_UWord16 length,
                                        const WebRtc_UWord8* data) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::OnApplicationDataReceived(id=%d, subType=%u,"
               " name=%u, length=%u)",
               id, subType, name, length);
}

WebRtc_Word32 Channel::OnInitializeDecoder(
  const WebRtc_Word32 id,
  const WebRtc_Word8 payloadType,
  const char payloadName[RTP_PAYLOAD_NAME_SIZE],
  const int frequency,
  const WebRtc_UWord8 channels,
  const WebRtc_UWord32 rate) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::OnInitializeDecoder(id=%d, payloadType=%d, "
               "payloadName=%s, frequency=%u, channels=%u, rate=%u)",
               id, payloadType, payloadName, frequency, channels, rate);

  assert(VoEChannelId(id) == _channelId);

  CodecInst receiveCodec = {0};
  CodecInst dummyCodec = {0};

  receiveCodec.pltype = payloadType;
  receiveCodec.plfreq = frequency;
  receiveCodec.channels = channels;
  receiveCodec.rate = rate;
  strncpy(receiveCodec.plname, payloadName, RTP_PAYLOAD_NAME_SIZE - 1);

  _audioCodingModule.Codec(payloadName, dummyCodec, frequency, channels);
  receiveCodec.pacsize = dummyCodec.pacsize;

  // Register the new codec to the ACM
  if (_audioCodingModule.RegisterReceiveCodec(receiveCodec) == -1) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "Channel::OnInitializeDecoder() invalid codec ("
                 "pt=%d, name=%s) received - 1", payloadType, payloadName);
    _engineStatisticsPtr->SetLastError(VE_AUDIO_CODING_MODULE_ERROR);
    return -1;
  }

  return 0;
}

void Channel::OnPacketTimeout(const WebRtc_Word32 id) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::OnPacketTimeout(id=%d)", id);

  CriticalSectionScoped cs(_callbackCritSectPtr);

  if (_voiceEngineObserverPtr) {
    if (_receiving || _externalTransport) {
      WebRtc_Word32 channel = VoEChannelId(id);
      assert(channel == _channelId);
      // Ensure that next OnReceivedPacket() callback will trigger
      // a VE_PACKET_RECEIPT_RESTARTED callback.
      _rtpPacketTimedOut = true;
      // Deliver callback to the observer
      WEBRTC_TRACE(kTraceInfo, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::OnPacketTimeout() => "
                   "CallbackOnError(VE_RECEIVE_PACKET_TIMEOUT)");
      /*_voiceEngineObserverPtr->CallbackOnError(channel,
        VE_RECEIVE_PACKET_TIMEOUT);*/
    }
  }
}

void Channel::OnReceivedPacket(const WebRtc_Word32 id,
                               const RtpRtcpPacketType packetType) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::OnReceivedPacket(id=%d, packetType=%d)",
               id, packetType);

  assert(VoEChannelId(id) == _channelId);

  // Notify only for the case when we have restarted an RTP session.
  if (_rtpPacketTimedOut && (kPacketRtp == packetType)) {
    CriticalSectionScoped cs(_callbackCritSectPtr);

    if (_voiceEngineObserverPtr) {
      WebRtc_Word32 channel = VoEChannelId(id);
      assert(channel == _channelId);
      // Reset timeout mechanism
      _rtpPacketTimedOut = false;
      // Deliver callback to the observer
      WEBRTC_TRACE(kTraceInfo, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::OnPacketTimeout() =>"
                   " CallbackOnError(VE_PACKET_RECEIPT_RESTARTED)");
      /*_voiceEngineObserverPtr->CallbackOnError(
        channel,
        VE_PACKET_RECEIPT_RESTARTED);*/
    }
  }
}

void Channel::OnPeriodicDeadOrAlive(const WebRtc_Word32 id,
                                    const RTPAliveType alive) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::OnPeriodicDeadOrAlive(id=%d, alive=%d)", id, alive);

  if (!_connectionObserver)
    return;

  WebRtc_Word32 channel = VoEChannelId(id);
  assert(channel == _channelId);

  // Use Alive as default to limit risk of false Dead detections
  bool isAlive(true);

  // Always mark the connection as Dead when the module reports kRtpDead
  if (kRtpDead == alive) {
    isAlive = false;
  }

  // It is possible that the connection is alive even if no RTP packet has
  // been received for a long time since the other side might use VAD/DTX
  // and a low SID-packet update rate.
  if ((kRtpNoRtp == alive) && _playing) {
    // Detect Alive for all NetEQ states except for the case when we are
    // in PLC_CNG state.
    // PLC_CNG <=> background noise only due to long expand or error.
    // Note that, the case where the other side stops sending during CNG
    // state will be detected as Alive. Dead is is not set until after
    // missing RTCP packets for at least twelve seconds (handled
    // internally by the RTP/RTCP module).
    isAlive = (_outputSpeechType != AudioFrame::kPLCCNG);
  }

  UpdateDeadOrAliveCounters(isAlive);

  // Send callback to the registered observer
  if (_connectionObserver) {
    CriticalSectionScoped cs(&_callbackCritSect);

    if (_connectionObserverPtr) {
      _connectionObserverPtr->OnPeriodicDeadOrAlive(channel, isAlive);
    }
  }
}

WebRtc_Word32 Channel::OnReceivedPayloadData(const WebRtc_UWord8* payloadData,
    const WebRtc_UWord16 payloadSize,
    const WebRtcRTPHeader* rtpHeader) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::OnReceivedPayloadData(payloadSize=%d,"
               " payloadType=%u, audioChannel=%u)",
               payloadSize,
               rtpHeader->header.payloadType,
               rtpHeader->type.Audio.channel);

  if (!_playing) {
    // Avoid inserting into NetEQ when we are not playing. Count the
    // packet as discarded.
    WEBRTC_TRACE(kTraceStream, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "received packet is discarded since playing is not"
                 " activated");
    _numberOfDiscardedPackets++;
    return 0;
  }

  // Push the incoming payload (parsed and ready for decoding) into the ACM
  if (_audioCodingModule.IncomingPacket(payloadData,
                                        payloadSize, *rtpHeader) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CODING_MODULE_ERROR, kTraceWarning,
      "Channel::OnReceivedPayloadData() unable to push data to the ACM");
    return -1;
  }

  // Update the packet delay
  UpdatePacketDelay(rtpHeader->header.timestamp,
                    rtpHeader->header.sequenceNumber);

  return 0;
}

WebRtc_Word32 Channel::GetAudioFrame(const WebRtc_Word32 id,
                                     AudioFrame& audioFrame) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetAudioFrame(id=%d)", id);

  // Get 10ms raw PCM data from the ACM (mixer limits output frequency)
  if (_audioCodingModule.PlayoutData10Ms(audioFrame.sample_rate_hz_,
                                         audioFrame) == -1) {
    WEBRTC_TRACE(kTraceError, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "Channel::GetAudioFrame() PlayoutData10Ms() failed!");
    // In all likelihood, the audio in this frame is garbage. We return an
    // error so that the audio mixer module doesn't add it to the mix. As
    // a result, it won't be played out and the actions skipped here are
    // irrelevant.
    return -1;
  }

  if (_RxVadDetection) {
    UpdateRxVadDetection(audioFrame);
  }

  // Convert module ID to internal VoE channel ID
  audioFrame.id_ = VoEChannelId(audioFrame.id_);
  // Store speech type for dead-or-alive detection
  _outputSpeechType = audioFrame.speech_type_;

  // Perform far-end AudioProcessing module processing on the received signal
  if (_rxApmIsEnabled) {
    ApmProcessRx(audioFrame);
  }

  // Output volume scaling
  if (_outputGain < 0.99f || _outputGain > 1.01f) {
    AudioFrameOperations::ScaleWithSat(_outputGain, audioFrame);
  }

  // Scale left and/or right channel(s) if stereo and master balance is
  // active

  if (_panLeft != 1.0f || _panRight != 1.0f) {
    if (audioFrame.num_channels_ == 1) {
      // Emulate stereo mode since panning is active.
      // The mono signal is copied to both left and right channels here.
      AudioFrameOperations::MonoToStereo(&audioFrame);
    }

    // For true stereo mode (when we are receiving a stereo signal), no
    // action is needed.

    // Do the panning operation (the audio frame contains stereo at this
    // stage)
    AudioFrameOperations::Scale(_panLeft, _panRight, audioFrame);
  }

  // Mix decoded PCM output with file if file mixing is enabled
  if (_outputFilePlaying) {
    MixAudioWithFile(audioFrame, audioFrame.sample_rate_hz_);
  }

  // Place channel in on-hold state (~muted) if on-hold is activated
  if (_outputIsOnHold) {
    AudioFrameOperations::Mute(audioFrame);
  }

  // External media
  if (_outputExternalMedia) {
    CriticalSectionScoped cs(&_callbackCritSect);
    const bool isStereo = (audioFrame.num_channels_ == 2);

    if (_outputExternalMediaCallbackPtr) {
      _outputExternalMediaCallbackPtr->Process(
        _channelId,
        kPlaybackPerChannel,
        (WebRtc_Word16*)audioFrame.data_,
        audioFrame.samples_per_channel_,
        audioFrame.sample_rate_hz_,
        isStereo);
    }
  }

  // Record playout if enabled
  {
    CriticalSectionScoped cs(&_fileCritSect);

    if (_outputFileRecording && _outputFileRecorderPtr) {
      _outputFileRecorderPtr->RecordAudioToFile(audioFrame);
    }
  }

  // Measure audio level (0-9)
  _outputAudioLevel.ComputeLevel(audioFrame);

  return 0;
}

WebRtc_Word32 Channel::NeededFrequency(const WebRtc_Word32 id) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::NeededFrequency(id=%d)", id);
  int highestNeeded = 0;

  // Determine highest needed receive frequency
  WebRtc_Word32 receiveFrequency = _audioCodingModule.ReceiveFrequency();

  // Return the bigger of playout and receive frequency in the ACM.
  if (_audioCodingModule.PlayoutFrequency() > receiveFrequency) {
    highestNeeded = _audioCodingModule.PlayoutFrequency();
  } else {
    highestNeeded = receiveFrequency;
  }

  // Special case, if we're playing a file on the playout side
  // we take that frequency into consideration as well
  // This is not needed on sending side, since the codec will
  // limit the spectrum anyway.
  if (_outputFilePlaying) {
    CriticalSectionScoped cs(&_fileCritSect);

    if (_outputFilePlayerPtr && _outputFilePlaying) {
      if(_outputFilePlayerPtr->Frequency() > highestNeeded) {
        highestNeeded = _outputFilePlayerPtr->Frequency();
      }
    }
  }

  return(highestNeeded);
}

WebRtc_Word32
Channel::UpdatePacketDelay(const WebRtc_UWord32 timestamp,
                           const WebRtc_UWord16 sequenceNumber) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::UpdatePacketDelay(timestamp=%lu, sequenceNumber=%u)",
               timestamp, sequenceNumber);

  WebRtc_Word32 rtpReceiveFrequency(0);

  // Get frequency of last received payload
  rtpReceiveFrequency = _audioCodingModule.ReceiveFrequency();

  CodecInst currRecCodec;

  if (_audioCodingModule.ReceiveCodec(currRecCodec) == 0) {
    if (STR_CASE_CMP("G722", currRecCodec.plname) == 0) {
      // Even though the actual sampling rate for G.722 audio is
      // 16,000 Hz, the RTP clock rate for the G722 payload format is
      // 8,000 Hz because that value was erroneously assigned in
      // RFC 1890 and must remain unchanged for backward compatibility.
      rtpReceiveFrequency = 8000;
    } else if (STR_CASE_CMP("opus", currRecCodec.plname) == 0) {
      // We are resampling Opus internally to 32,000 Hz until all our
      // DSP routines can operate at 48,000 Hz, but the RTP clock
      // rate for the Opus payload format is standardized to 48,000 Hz,
      // because that is the maximum supported decoding sampling rate.
      rtpReceiveFrequency = 48000;
    }
  }

  const WebRtc_UWord32 timeStampDiff = timestamp - _playoutTimeStampRTP;
  WebRtc_UWord32 timeStampDiffMs(0);

  if (timeStampDiff > 0) {
    switch (rtpReceiveFrequency) {
      case 8000:
        timeStampDiffMs = static_cast<WebRtc_UWord32>(timeStampDiff >> 3);
        break;

      case 16000:
        timeStampDiffMs = static_cast<WebRtc_UWord32>(timeStampDiff >> 4);
        break;

      case 32000:
        timeStampDiffMs = static_cast<WebRtc_UWord32>(timeStampDiff >> 5);
        break;

      case 48000:
        timeStampDiffMs = static_cast<WebRtc_UWord32>(timeStampDiff / 48);
        break;

      default:
        WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                     VoEId(_instanceId, _channelId),
                     "Channel::UpdatePacketDelay() invalid sample rate");
        timeStampDiffMs = 0;
        return -1;
    }

    if (timeStampDiffMs > 5000) {
      timeStampDiffMs = 0;
    }

    if (_averageDelayMs == 0) {
      _averageDelayMs = timeStampDiffMs;
    } else {
      // Filter average delay value using exponential filter (alpha is
      // 7/8). We derive 10*_averageDelayMs here (reduces risk of
      // rounding error) and compensate for it in GetDelayEstimate()
      // later. Adding 4/8 results in correct rounding.
      _averageDelayMs = ((_averageDelayMs * 7 + 10 * timeStampDiffMs + 4) >> 3);
    }

    if (sequenceNumber - _previousSequenceNumber == 1) {
      WebRtc_UWord16 packetDelayMs = 0;

      switch (rtpReceiveFrequency) {
        case 8000:
          packetDelayMs = static_cast<WebRtc_UWord16>(
                            (timestamp - _previousTimestamp) >> 3);
          break;

        case 16000:
          packetDelayMs = static_cast<WebRtc_UWord16>(
                            (timestamp - _previousTimestamp) >> 4);
          break;

        case 32000:
          packetDelayMs = static_cast<WebRtc_UWord16>(
                            (timestamp - _previousTimestamp) >> 5);
          break;

        case 48000:
          packetDelayMs = static_cast<WebRtc_UWord16>(
                            (timestamp - _previousTimestamp) / 48);
          break;
      }

      if (packetDelayMs >= 10 && packetDelayMs <= 60)
        _recPacketDelayMs = packetDelayMs;
    }
  }

  _previousSequenceNumber = sequenceNumber;
  _previousTimestamp = timestamp;

  return 0;
}

void Channel::RegisterReceiveCodecsToRTPModule() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::RegisterReceiveCodecsToRTPModule()");


  CodecInst codec;
  const WebRtc_UWord8 nSupportedCodecs = AudioCodingModule::NumberOfCodecs();

  for (int idx = 0; idx < nSupportedCodecs; idx++) {
    // Open up the RTP/RTCP receiver for all supported codecs
    if ((_audioCodingModule.Codec(idx, codec) == -1) ||
        (_rtpRtcpModule->RegisterReceivePayload(codec) == -1)) {
      WEBRTC_TRACE(
        kTraceWarning,
        kTraceVoice,
        VoEId(_instanceId, _channelId),
        "Channel::RegisterReceiveCodecsToRTPModule() unable"
        " to register %s (%d/%d/%d/%d) to RTP/RTCP receiver",
        codec.plname, codec.pltype, codec.plfreq,
        codec.channels, codec.rate);
    } else {
      WEBRTC_TRACE(
        kTraceInfo,
        kTraceVoice,
        VoEId(_instanceId, _channelId),
        "Channel::RegisterReceiveCodecsToRTPModule() %s "
        "(%d/%d/%d/%d) has been added to the RTP/RTCP "
        "receiver",
        codec.plname, codec.pltype, codec.plfreq,
        codec.channels, codec.rate);
    }
  }
}

void Channel::PlayNotification(const WebRtc_Word32 id,
                               const WebRtc_UWord32 durationMs) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::PlayNotification(id=%d, durationMs=%d)",
               id, durationMs);

  // Not implement yet
}

void Channel::RecordNotification(const WebRtc_Word32 id,
                                 const WebRtc_UWord32 durationMs) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::RecordNotification(id=%d, durationMs=%d)",
               id, durationMs);

  // Not implement yet
}

void Channel::PlayFileEnded(const WebRtc_Word32 id) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::PlayFileEnded(id=%d)", id);
}

void Channel::RecordFileEnded(const WebRtc_Word32 id) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::RecordFileEnded(id=%d)", id);
}


WebRtc_Word32 Channel::CreateChannel(Channel*& channel,
                                     const WebRtc_Word32 channelId,
                                     const WebRtc_UWord32 instanceId) {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(instanceId, channelId),
               "Channel::CreateChannel(channelId=%d, instanceId=%d)",
               channelId, instanceId);

  channel = new Channel(channelId, instanceId);

  if (channel == NULL) {
    WEBRTC_TRACE(kTraceMemory, kTraceVoice,
                 VoEId(instanceId, channelId),
                 "Channel::CreateChannel() unable to allocate memory for"
                 " channel");
    return -1;
  }

  //StartRTPDump()
  return 0;
}


Channel::Channel(const WebRtc_Word32 channelId,
                 const WebRtc_UWord32 instanceId) :
  _fileCritSect(*CriticalSectionWrapper::CreateCriticalSection()),
  _callbackCritSect(*CriticalSectionWrapper::CreateCriticalSection()),
  _instanceId(instanceId),
  _channelId(channelId),
  _audioCodingModule(*AudioCodingModule::Create(VoEModuleId(instanceId,
                     channelId))),
  _rtpDumpIn(*RtpDump::CreateRtpDump()),
  _rtpDumpOut(*RtpDump::CreateRtpDump()),
  _outputAudioLevel(),
  _externalTransport(false),
  _inputFilePlayerPtr(NULL),
  _outputFilePlayerPtr(NULL),
  _outputFileRecorderPtr(NULL),
  // Avoid conflict with other channels by adding 1024 - 1026,
  // won't use as much as 1024 channels.
  _inputFilePlayerId(VoEModuleId(instanceId, channelId) + 1024),
  _outputFilePlayerId(VoEModuleId(instanceId, channelId) + 1025),
  _outputFileRecorderId(VoEModuleId(instanceId, channelId) + 1026),
  _inputFilePlaying(false),
  _outputFilePlaying(false),
  _outputFileRecording(false),
  _inputExternalMedia(false),
  _outputExternalMedia(false),
  _inputExternalMediaCallbackPtr(NULL),
  _outputExternalMediaCallbackPtr(NULL),
  _encryptionRTPBufferPtr(NULL),
  _decryptionRTPBufferPtr(NULL),
  _encryptionRTCPBufferPtr(NULL),
  _decryptionRTCPBufferPtr(NULL),
  _timeStamp(
    0), // This is just an offset, RTP module will add it's own random offset
  _sendTelephoneEventPayloadType(106),
  _playoutTimeStampRTP(0),
  _playoutTimeStampRTCP(0),
  _numberOfDiscardedPackets(0),
  _engineStatisticsPtr(NULL),
  _outputMixerPtr(NULL),
  _transmitMixerPtr(NULL),
  _moduleProcessThreadPtr(NULL),
  _audioDeviceModulePtr(NULL),
  _voiceEngineObserverPtr(NULL),
  _callbackCritSectPtr(NULL),
  _transportPtr(NULL),
  _encryptionPtr(NULL),
  _rtpAudioProc(NULL),
  _rxAudioProcessingModulePtr(NULL),
  _enablePacketCombine(false),
  //_rxVadObserverPtr(NULL),
  _dataCombinePtr(NULL),
  _enablePacketSeparate(false),
  _dataSeparatorPtr(NULL),
  _oldVadDecision(-1),
  _sendFrameType(0),
  _rtpObserverPtr(NULL),
  _rtcpObserverPtr(NULL),
  _outputIsOnHold(false),
  _externalPlayout(false),
  _inputIsOnHold(false),
  _playing(false),
  _sending(false),
  _receiving(false),
  _mixFileWithMicrophone(false),
  _rtpObserver(false),
  _rtcpObserver(false),
  _mute(false),
  _panLeft(1.0f),
  _panRight(1.0f),
  _outputGain(1.0f),
  _encrypting(false),
  _decrypting(false),
  _playOutbandDtmfEvent(false),
  _playInbandDtmfEvent(false),
  _inbandTelephoneEventDetection(false),
  _outOfBandTelephoneEventDetecion(false),
  _extraPayloadType(0),
  _insertExtraRTPPacket(false),
  _extraMarkerBit(false),
  _lastLocalTimeStamp(0),
  _lastPayloadType(0),
  _includeAudioLevelIndication(false),
  _rtpPacketTimedOut(false),
  _rtpPacketTimeOutIsEnabled(false),
  _rtpTimeOutSeconds(0),
  _connectionObserver(false),
  _connectionObserverPtr(NULL),
  _countAliveDetections(0),
  _countDeadDetections(0),
  _outputSpeechType(AudioFrame::kNormalSpeech),
  _averageDelayMs(0),
  _previousSequenceNumber(0),
  _previousTimestamp(0),
  _recPacketDelayMs(20),
  _RxVadDetection(false),
  _rxApmIsEnabled(false),
  _rxAgcIsEnabled(false),
  _rxNsIsEnabled(false),
  _cntPerMs(0) {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::Channel() - ctor");

  RtpRtcp::Configuration configuration;
  configuration.id = VoEModuleId(instanceId, channelId);
  configuration.audio = true;
  configuration.incoming_data = this;
  configuration.incoming_messages = this;
  configuration.outgoing_transport = this;
  configuration.rtcp_feedback = this;
  configuration.audio_messages = this;

  _rtpRtcpModule.reset(RtpRtcp::CreateRtpRtcp(configuration));

  _audioCodingModule.SetBackgroundNoiseMode(Fade);

  // Create far end AudioProcessing Module
  _rxAudioProcessingModulePtr = AudioProcessing::Create(
                                  VoEModuleId(instanceId, channelId));
}

Channel::~Channel() {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::~Channel() - dtor");

  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "_ModuleProcessThreadPtr(%08x), _rtpRtcpModule(%08x)",
               _moduleProcessThreadPtr, _rtpRtcpModule.get());

  if (_outputExternalMedia) {
    DeRegisterExternalMediaProcessing(kPlaybackPerChannel);
  }

  if (_inputExternalMedia) {
    DeRegisterExternalMediaProcessing(kRecordingPerChannel);
  }

  StopSend();
//#ifndef WEBRTC_EXTERNAL_TRANSPORT
//  StopReceiving();
//  // De-register packet callback to ensure we're not in a callback when
//  // deleting channel state, avoids race condition and deadlock.
//  if (_socketTransportModule.InitializeReceiveSockets(NULL, 0, NULL, NULL, 0)
//    != 0)
//  {
//    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
//      VoEId(_instanceId, _channelId),
//      "~Channel() failed to de-register receive callback");
//  }
//#endif
  StopPlayout();

  {
    CriticalSectionScoped cs(&_fileCritSect);

    if (_inputFilePlayerPtr) {
      _inputFilePlayerPtr->RegisterModuleFileCallback(NULL);
      _inputFilePlayerPtr->StopPlayingFile();
      FilePlayer::DestroyFilePlayer(_inputFilePlayerPtr);
      _inputFilePlayerPtr = NULL;
    }

    if (_outputFilePlayerPtr) {
      _outputFilePlayerPtr->RegisterModuleFileCallback(NULL);
      _outputFilePlayerPtr->StopPlayingFile();
      FilePlayer::DestroyFilePlayer(_outputFilePlayerPtr);
      _outputFilePlayerPtr = NULL;
    }

    if (_outputFileRecorderPtr) {
      _outputFileRecorderPtr->RegisterModuleFileCallback(NULL);
      _outputFileRecorderPtr->StopRecording();
      FileRecorder::DestroyFileRecorder(_outputFileRecorderPtr);
      _outputFileRecorderPtr = NULL;
    }
  }

  // The order to safely shutdown modules in a channel is:
  // 1. De-register callbacks in modules
  // 2. De-register modules in process thread
  // 3. Destroy modules
  if (_audioCodingModule.RegisterTransportCallback(NULL) == -1) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "~Channel() failed to de-register transport callback"
                 " (Audio coding module)");
  }

  if (_audioCodingModule.RegisterVADCallback(NULL) == -1) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "~Channel() failed to de-register VAD callback"
                 " (Audio coding module)");
  }

#ifdef WEBRTC_DTMF_DETECTION

  if (_audioCodingModule.RegisterIncomingMessagesCallback(NULL) == -1) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "~Channel() failed to de-register incoming messages "
                 "callback (Audio coding module)");
  }

#endif
  // De-register modules in process thread
//#ifndef WEBRTC_EXTERNAL_TRANSPORT
//  if (_moduleProcessThreadPtr->DeRegisterModule(&_socketTransportModule)
//    == -1)
//  {
//    WEBRTC_TRACE(kTraceInfo, kTraceVoice,
//      VoEId(_instanceId,_channelId),
//      "~Channel() failed to deregister socket module");
//  }
//#endif

  if (_moduleProcessThreadPtr->DeRegisterModule(_rtpRtcpModule.get()) == -1) {
    WEBRTC_TRACE(kTraceInfo, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "~Channel() failed to deregister RTP/RTCP module");
  }

  // Destroy modules
//#ifndef WEBRTC_EXTERNAL_TRANSPORT
//  UdpTransport::Destroy(
//    &_socketTransportModule);
//#endif
  AudioCodingModule::Destroy(&_audioCodingModule);
#ifdef WEBRTC_SRTP
  SrtpModule::DestroySrtpModule(&_srtpModule);
#endif

  if (_rxAudioProcessingModulePtr != NULL) {
    AudioProcessing::Destroy(_rxAudioProcessingModulePtr); // far end APM
    _rxAudioProcessingModulePtr = NULL;
  }

  // End of modules shutdown

  if (_dataCombinePtr) {
    delete _dataCombinePtr;
  }

  if (_dataSeparatorPtr) {
    delete _dataSeparatorPtr;
  }

  // Delete other objects
  RtpDump::DestroyRtpDump(&_rtpDumpIn);
  RtpDump::DestroyRtpDump(&_rtpDumpOut);
  delete [] _encryptionRTPBufferPtr;
  delete [] _decryptionRTPBufferPtr;
  delete [] _encryptionRTCPBufferPtr;
  delete [] _decryptionRTCPBufferPtr;
  delete &_callbackCritSect;
  delete &_fileCritSect;
}

WebRtc_Word32 Channel::Init() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::Init()");

  // --- Initial sanity

  if ((_engineStatisticsPtr == NULL) || (_moduleProcessThreadPtr == NULL)) {
    WEBRTC_TRACE(kTraceError, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "Channel::Init() must call SetEngineInformation() first");
    return -1;
  }

  // --- Add modules to process thread (for periodic schedulation)

  const bool processThreadFail =
    ((_moduleProcessThreadPtr->RegisterModule(_rtpRtcpModule.get()) != 0) || false);

  if (processThreadFail) {
    _engineStatisticsPtr->SetLastError(
      VE_CANNOT_INIT_CHANNEL, kTraceError,
      "Channel::Init() modules not registered");
    return -1;
  }

  // --- ACM initialization

  if ((_audioCodingModule.InitializeReceiver() == -1) ||
//#ifdef WEBRTC_CODEC_AVT
//    // out-of-band Dtmf tones are played out by default
//    (_audioCodingModule.SetDtmfPlayoutStatus(true) == -1) ||
//#endif
      (_audioCodingModule.InitializeSender() == -1)) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CODING_MODULE_ERROR, kTraceError,
      "Channel::Init() unable to initialize the ACM - 1");
    return -1;
  }

  // --- RTP/RTCP module initialization

  // Ensure that RTCP is enabled by default for the created channel.
  // Note that, the module will keep generating RTCP until it is explicitly
  // disabled by the user.
  // After StopListen (when no sockets exists), RTCP packets will no longer
  // be transmitted since the Transport object will then be invalid.

  //const bool rtpRtcpFail =
  //  ((_rtpRtcpModule->SetTelephoneEventStatus(false, true, true) == -1) ||
  //  // RTCP is enabled by default
  //  (_rtpRtcpModule->SetRTCPStatus(kRtcpCompound) == -1));
  //if (rtpRtcpFail)
  //{
  //  _engineStatisticsPtr->SetLastError(
  //    VE_RTP_RTCP_MODULE_ERROR, kTraceError,
  //    "Channel::Init() RTP/RTCP module not initialized");
  //  return -1;
  //}

  // --- Register all permanent callbacks
  const bool fail = (_audioCodingModule.RegisterTransportCallback(this) == -1) ||
                    (_audioCodingModule.RegisterVADCallback(this) == -1);

  if (fail) {
    _engineStatisticsPtr->SetLastError(
      VE_CANNOT_INIT_CHANNEL, kTraceError,
      "Channel::Init() callbacks not registered");
    return -1;
  }

  // --- Register all supported codecs to the receiving side of the
  // RTP/RTCP module

  CodecInst codec;
  const WebRtc_UWord8 nSupportedCodecs = AudioCodingModule::NumberOfCodecs();

  for (int idx = 0; idx < nSupportedCodecs; idx++) {
    // Open up the RTP/RTCP receiver for all supported codecs
    if ((_audioCodingModule.Codec(idx, codec) == -1) ||
        (_rtpRtcpModule->RegisterReceivePayload(codec) == -1)) {
      WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::Init() unable to register %s (%d/%d/%d/%d) "
                   "to RTP/RTCP receiver",
                   codec.plname, codec.pltype, codec.plfreq,
                   codec.channels, codec.rate);
    } else {
      WEBRTC_TRACE(kTraceInfo, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::Init() %s (%d/%d/%d/%d) has been added to "
                   "the RTP/RTCP receiver",
                   codec.plname, codec.pltype, codec.plfreq,
                   codec.channels, codec.rate);
    }

    // Ensure that PCMU is used as default codec on the sending side
    if (!STR_CASE_CMP(codec.plname, "PCMU") && (codec.channels == 1)) {
      SetSendCodec(codec);
    }

    // Register default PT for outband 'telephone-event'
    if (!STR_CASE_CMP(codec.plname, "telephone-event")) {
      if ((_rtpRtcpModule->RegisterSendPayload(codec) == -1) ||
          (_audioCodingModule.RegisterReceiveCodec(codec) == -1)) {
        WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                     VoEId(_instanceId, _channelId),
                     "Channel::Init() failed to register outband "
                     "'telephone-event' (%d/%d) correctly",
                     codec.pltype, codec.plfreq);
      }
    }

    if (!STR_CASE_CMP(codec.plname, "CN")) {
      if ((_audioCodingModule.RegisterSendCodec(codec) == -1) ||
          (_audioCodingModule.RegisterReceiveCodec(codec) == -1) ||
          (_rtpRtcpModule->RegisterSendPayload(codec) == -1)) {
        WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                     VoEId(_instanceId, _channelId),
                     "Channel::Init() failed to register CN (%d/%d) "
                     "correctly - 1",
                     codec.pltype, codec.plfreq);
      }
    }

#ifdef WEBRTC_CODEC_RED

    // Register RED to the receiving side of the ACM.
    // We will not receive an OnInitializeDecoder() callback for RED.
    if (!STR_CASE_CMP(codec.plname, "RED")) {
      if (_audioCodingModule.RegisterReceiveCodec(codec) == -1) {
        WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                     VoEId(_instanceId, _channelId),
                     "Channel::Init() failed to register RED (%d/%d) "
                     "correctly",
                     codec.pltype, codec.plfreq);
      }
    }

#endif
  }


  // Initialize the far end AP module
  // Using 8 kHz as initial Fs, the same as in transmission. Might be
  // changed at the first receiving audio.
  if (_rxAudioProcessingModulePtr == NULL) {
    _engineStatisticsPtr->SetLastError(
      VE_NO_MEMORY, kTraceCritical,
      "Channel::Init() failed to create the far-end AudioProcessing"
      " module");
    return -1;
  }

  if (_rxAudioProcessingModulePtr->set_sample_rate_hz(8000)) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceWarning,
      "Channel::Init() failed to set the sample rate to 8K for"
      " far-end AP module");
  }

  if (_rxAudioProcessingModulePtr->set_num_channels(1, 1) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_SOUNDCARD_ERROR, kTraceWarning,
      "Init() failed to set channels for the primary audio stream");
  }

  if (_rxAudioProcessingModulePtr->high_pass_filter()->Enable(
        WEBRTC_VOICE_ENGINE_RX_HP_DEFAULT_STATE) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceWarning,
      "Channel::Init() failed to set the high-pass filter for"
      " far-end AP module");
  }

  if (_rxAudioProcessingModulePtr->noise_suppression()->set_level(
        (NoiseSuppression::Level)WEBRTC_VOICE_ENGINE_RX_NS_DEFAULT_MODE) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceWarning,
      "Init() failed to set noise reduction level for far-end"
      " AP module");
  }

  if (_rxAudioProcessingModulePtr->noise_suppression()->Enable(
        WEBRTC_VOICE_ENGINE_RX_NS_DEFAULT_STATE) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceWarning,
      "Init() failed to set noise reduction state for far-end"
      " AP module");
  }

  if (_rxAudioProcessingModulePtr->gain_control()->set_mode(
        (GainControl::Mode)WEBRTC_VOICE_ENGINE_RX_AGC_DEFAULT_MODE) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceWarning,
      "Init() failed to set AGC mode for far-end AP module");
  }

  if (_rxAudioProcessingModulePtr->gain_control()->Enable(
        WEBRTC_VOICE_ENGINE_RX_AGC_DEFAULT_STATE) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceWarning,
      "Init() failed to set AGC state for far-end AP module");
  }

  return 0;
}

WebRtc_Word32 Channel::SetEngineInformation(Statistics& engineStatistics,
    OutputMixer& outputMixer,
    voe::TransmitMixer& transmitMixer,
    ProcessThread& moduleProcessThread,
    AudioDeviceModule& audioDeviceModule,
    VoiceEngineObserver* voiceEngineObserver,
    CriticalSectionWrapper* callbackCritSect) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetEngineInformation()");
  _engineStatisticsPtr = &engineStatistics;
  _outputMixerPtr = &outputMixer;
  _transmitMixerPtr = &transmitMixer,
  _moduleProcessThreadPtr = &moduleProcessThread;
  _audioDeviceModulePtr = &audioDeviceModule;
  _voiceEngineObserverPtr = voiceEngineObserver;
  _callbackCritSectPtr = callbackCritSect;
  return 0;
}

WebRtc_Word32 Channel::GetSendCodec(CodecInst& codec) {
  return (_audioCodingModule.SendCodec(codec));
}

WebRtc_Word32 Channel::GetRecCodec(CodecInst& codec) {
  return (_audioCodingModule.ReceiveCodec(codec));
}

WebRtc_Word32 Channel::SetSendCodec(const CodecInst& codec) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetSendCodec()");

  if (_audioCodingModule.RegisterSendCodec(codec) != 0) {
    WEBRTC_TRACE(kTraceError, kTraceVoice, VoEId(_instanceId, _channelId),
                 "SetSendCodec() failed to register codec to ACM");
    return -1;
  }

  if (_rtpRtcpModule->RegisterSendPayload(codec) != 0) {
    _rtpRtcpModule->DeRegisterSendPayload(codec.pltype);

    if (_rtpRtcpModule->RegisterSendPayload(codec) != 0) {
      WEBRTC_TRACE(
        kTraceError, kTraceVoice, VoEId(_instanceId, _channelId),
        "SetSendCodec() failed to register codec to"
        " RTP/RTCP module");
      return -1;
    }
  }

  if (_rtpRtcpModule->SetAudioPacketSize(codec.pacsize) != 0) {
    WEBRTC_TRACE(kTraceError, kTraceVoice, VoEId(_instanceId, _channelId),
                 "SetSendCodec() failed to set audio packet size");
    return -1;
  }/**/

  _rxCodec = codec;
  _cntPerMs = _rxCodec.rate * _rxCodec.channels / 1000;
  return 0;
}

WebRtc_Word32
Channel::SetVADStatus(bool enableVAD, ACMVADMode mode, bool disableDTX) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetVADStatus(mode=%d)", mode);
  // To disable VAD, DTX must be disabled too
  disableDTX = ((enableVAD == false) ? true : disableDTX);

  if (_audioCodingModule.SetVAD(!disableDTX, enableVAD, mode) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CODING_MODULE_ERROR, kTraceError,
      "SetVADStatus() failed to set VAD");
    return -1;
  }

  return 0;
}

WebRtc_Word32
Channel::GetVADStatus(bool& enabledVAD, ACMVADMode& mode, bool& disabledDTX) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetVADStatus");

  if (_audioCodingModule.VAD(disabledDTX, enabledVAD, mode) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CODING_MODULE_ERROR, kTraceError,
      "GetVADStatus() failed to get VAD status");
    return -1;
  }

  disabledDTX = !disabledDTX;
  return 0;
}

WebRtc_Word32 Channel::UpdateLocalTimeStamp() {
  _timeStamp += _audioFrame.samples_per_channel_;
  return 0;
}


WebRtc_Word32 Channel::StartPlayout() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StartPlayout()");

  if (_playing) {
    return 0;
  }

  // Add participant as candidates for mixing.
  if (_outputMixerPtr->SetMixabilityStatus(*this, true) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CONF_MIX_MODULE_ERROR, kTraceError,
      "StartPlayout() failed to add participant to mixer");
    return -1;
  }

  _playing = true;

  if (RegisterFilePlayingToMixer() != 0)
    return -1;

  return 0;
}

WebRtc_Word32 Channel::StopPlayout() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StopPlayout()");

  if (!_playing) {
    return 0;
  }

  // Remove participant as candidates for mixing
  if (_outputMixerPtr->SetMixabilityStatus(*this, false) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CONF_MIX_MODULE_ERROR, kTraceError,
      "StartPlayout() failed to remove participant from mixer");
    return -1;
  }

  _playing = false;
  _outputAudioLevel.Clear();

  return 0;
}

WebRtc_Word32 Channel::StartSend() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StartSend()");
  {
    // A lock is needed because |_sending| can be accessed or modified bys
    // another thread at the same time.
    CriticalSectionScoped cs(&_callbackCritSect);

    if (_sending) {
      return 0;
    }

    _sending = true;
  }

  if (_rtpRtcpModule->SetSendingStatus(true) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_RTP_RTCP_MODULE_ERROR, kTraceError,
      "StartSend() RTP/RTCP failed to start sending");
    CriticalSectionScoped cs(&_callbackCritSect);
    _sending = false;
    return -1;
  }

  return 0;
}

WebRtc_Word32 Channel::StopSend() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StopSend()");

  {
    // A lock is needed because |_sending| can be accessed or modified by
    // another thread at the same time.
    CriticalSectionScoped cs(&_callbackCritSect);

    if (!_sending) {
      return 0;
    }

    _sending = false;
  }

  // Reset sending SSRC and sequence number and triggers direct transmission
  // of RTCP BYE
  if (_rtpRtcpModule->SetSendingStatus(false) == -1 ||
      _rtpRtcpModule->ResetSendDataCountersRTP() == -1) {
    _engineStatisticsPtr->SetLastError(
      VE_RTP_RTCP_MODULE_ERROR, kTraceWarning,
      "StartSend() RTP/RTCP failed to stop sending");
  }

  return 0;
}

WebRtc_Word32 Channel::StartReceiving() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StartReceiving()");

  if (_receiving) {
    return 0;
  }

  // If external transport is used, we will only initialize/set the variables
  // after this section, since we are not using the WebRtc transport but
  // still need to keep track of e.g. if we are receiving.
//#ifndef WEBRTC_EXTERNAL_TRANSPORT
//  if (!_externalTransport) {
//    if (!_socketTransportModule.ReceiveSocketsInitialized())
//    {
//      _engineStatisticsPtr->SetLastError(
//        VE_SOCKETS_NOT_INITED, kTraceError,
//        "StartReceive() must set local receiver first");
//      return -1;
//    }
//    if (_socketTransportModule.StartReceiving(KNumberOfSocketBuffers) != 0)
//    {
//      _engineStatisticsPtr->SetLastError(
//        VE_SOCKET_TRANSPORT_MODULE_ERROR, kTraceError,
//        "StartReceiving() failed to start receiving");
//      return -1;
//    }
//  }
//#endif
  _receiving = true;
  _numberOfDiscardedPackets = 0;

  return 0;
}

WebRtc_Word32 Channel::StopReceiving() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StopReceiving()");

  if (!_receiving) {
    return 0;
  }

//#ifndef WEBRTC_EXTERNAL_TRANSPORT
//  if (!_externalTransport &&
//    _socketTransportModule.ReceiveSocketsInitialized())
//  {
//    if (_socketTransportModule.StopReceiving() != 0)
//    {
//      _engineStatisticsPtr->SetLastError(
//        VE_SOCKET_TRANSPORT_MODULE_ERROR, kTraceError,
//        "StopReceiving() failed to stop receiving.");
//      return -1;
//    }
//  }
//#endif
  bool dtmfDetection = _rtpRtcpModule->TelephoneEvent();
  // Recover DTMF detection status.
  WebRtc_Word32 ret = _rtpRtcpModule->SetTelephoneEventStatus(dtmfDetection,
                      true, true);

  if (ret != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_OPERATION, kTraceWarning,
      "StopReceiving() failed to restore telephone-event status.");
  }

  RegisterReceiveCodecsToRTPModule();
  _receiving = false;
  return 0;
}

WebRtc_Word32 Channel::SetNetEQPlayoutMode(NetEqModes mode) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetNetEQPlayoutMode()");
  AudioPlayoutMode playoutMode(voice);

  switch (mode) {
    case kNetEqDefault:
      playoutMode = voice;
      break;

    case kNetEqStreaming:
      playoutMode = streaming;
      break;

    case kNetEqFax:
      playoutMode = fax;
      break;

    case kNetEqOff:
      playoutMode = off;
      break;
  }

  if (_audioCodingModule.SetPlayoutMode(playoutMode) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CODING_MODULE_ERROR, kTraceError,
      "SetNetEQPlayoutMode() failed to set playout mode");
    return -1;
  }

  return 0;
}

WebRtc_Word32 Channel::GetNetEQPlayoutMode(NetEqModes& mode) {
  const AudioPlayoutMode playoutMode = _audioCodingModule.PlayoutMode();

  switch (playoutMode) {
    case voice:
      mode = kNetEqDefault;
      break;

    case streaming:
      mode = kNetEqStreaming;
      break;

    case fax:
      mode = kNetEqFax;
      break;

    case off:
      mode = kNetEqOff;
  }

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "Channel::GetNetEQPlayoutMode() => mode=%u", mode);
  return 0;
}

WebRtc_Word32 Channel::GetNetEQBGNMode(NetEqBgnModes& mode) {
  ACMBackgroundNoiseMode noiseMode(On);
  _audioCodingModule.BackgroundNoiseMode(noiseMode);

  switch (noiseMode) {
    case On:
      mode = kBgnOn;
      break;

    case Fade:
      mode = kBgnFade;
      break;

    case Off:
      mode = kBgnOff;
      break;
  }

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetNetEQBGNMode() => mode=%u", mode);
  return 0;
}

WebRtc_Word32 Channel::SetNetEQBGNMode(NetEqBgnModes mode) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetNetEQPlayoutMode()");
  ACMBackgroundNoiseMode noiseMode(On);

  switch (mode) {
    case kBgnOn:
      noiseMode = On;
      break;

    case kBgnFade:
      noiseMode = Fade;
      break;

    case kBgnOff:
      noiseMode = Off;
      break;
  }

  if (_audioCodingModule.SetBackgroundNoiseMode(noiseMode) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CODING_MODULE_ERROR, kTraceError,
      "SetBackgroundNoiseMode() failed to set noise mode");
    return -1;
  }

  return 0;
}

WebRtc_Word32 Channel::SetOnHoldStatus(bool enable, OnHoldModes mode) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetOnHoldStatus()");

  if (mode == kHoldSendAndPlay) {
    _outputIsOnHold = enable;
    _inputIsOnHold = enable;
  } else if (mode == kHoldPlayOnly) {
    _outputIsOnHold = enable;
  }

  if (mode == kHoldSendOnly) {
    _inputIsOnHold = enable;
  }

  return 0;
}

WebRtc_Word32 Channel::GetOnHoldStatus(bool& enabled, OnHoldModes& mode) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetOnHoldStatus()");
  enabled = (_outputIsOnHold || _inputIsOnHold);

  if (_outputIsOnHold && _inputIsOnHold) {
    mode = kHoldSendAndPlay;
  } else if (_outputIsOnHold && !_inputIsOnHold) {
    mode = kHoldPlayOnly;
  } else if (!_outputIsOnHold && _inputIsOnHold) {
    mode = kHoldSendOnly;
  }

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetOnHoldStatus() => enabled=%d, mode=%d",
               enabled, mode);
  return 0;
}

// TODO(andrew): refactor Mix functions here and in transmit_mixer.cc to use
// a shared helper.
WebRtc_Word32 Channel::MixOrReplaceAudioWithFile(const int mixingFrequency) {
  scoped_array<WebRtc_Word16> fileBuffer(new WebRtc_Word16[640]);
  int fileSamples(0);

  {
    CriticalSectionScoped cs(&_fileCritSect);

    if (_inputFilePlayerPtr == NULL) {
      WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::MixOrReplaceAudioWithFile() fileplayer"
                   " doesnt exist");
      return -1;
    }

    if (_inputFilePlayerPtr->Get10msAudioFromFile(fileBuffer.get(),
        fileSamples,
        mixingFrequency) == -1) {
      WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::MixOrReplaceAudioWithFile() file mixing "
                   "failed");
      return -1;
    }

    if (fileSamples == 0) {
      WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::MixOrReplaceAudioWithFile() file is ended");
      return 0;
    }
  }

  assert(_audioFrame.samples_per_channel_ == fileSamples);

  if (_mixFileWithMicrophone) {
    // Currently file stream is always mono.
    // TODO(xians): Change the code when FilePlayer supports real stereo.
    Utility::MixWithSat(_audioFrame.data_,
                        _audioFrame.num_channels_,
                        fileBuffer.get(),
                        1,
                        fileSamples);
  } else {
    // Replace ACM audio with file.
    // Currently file stream is always mono.
    // TODO(xians): Change the code when FilePlayer supports real stereo.
    _audioFrame.UpdateFrame(_channelId,
                            -1,
                            fileBuffer.get(),
                            fileSamples,
                            mixingFrequency,
                            AudioFrame::kNormalSpeech,
                            AudioFrame::kVadUnknown,
                            1);

  }

  return 0;
}

WebRtc_Word32 Channel::MixAudioWithFile(AudioFrame& audioFrame,
                                        const int mixingFrequency) {
  assert(mixingFrequency <= 32000);

  scoped_array<WebRtc_Word16> fileBuffer(new WebRtc_Word16[640]);
  int fileSamples(0);

  {
    CriticalSectionScoped cs(&_fileCritSect);

    if (_outputFilePlayerPtr == NULL) {
      WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::MixAudioWithFile() file mixing failed");
      return -1;
    }

    // We should get the frequency we ask for.
    if (_outputFilePlayerPtr->Get10msAudioFromFile(fileBuffer.get(),
        fileSamples,
        mixingFrequency) == -1) {
      WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "Channel::MixAudioWithFile() file mixing failed");
      return -1;
    }
  }

  if (audioFrame.samples_per_channel_ == fileSamples) {
    // Currently file stream is always mono.
    // TODO(xians): Change the code when FilePlayer supports real stereo.
    Utility::MixWithSat(audioFrame.data_,
                        audioFrame.num_channels_,
                        fileBuffer.get(),
                        1,
                        fileSamples);
  } else {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice, VoEId(_instanceId, _channelId),
                 "Channel::MixAudioWithFile() samples_per_channel_(%d) != "
                 "fileSamples(%d)",
                 audioFrame.samples_per_channel_, fileSamples);
    return -1;
  }

  return 0;
}

WebRtc_Word32 Channel::GetPlayoutTimeStamp(WebRtc_UWord32& playoutTimestamp) {
  WebRtc_UWord32 timestamp(0);
  CodecInst currRecCodec;

  if (_audioCodingModule.PlayoutTimestamp(timestamp) == -1) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice, VoEId(_instanceId, _channelId),
                 "Channel::GetPlayoutTimeStamp() failed to read playout"
                 " timestamp from the ACM");
    return -1;
  }

  WebRtc_UWord16 delayMS(0);

  if (_audioDeviceModulePtr->PlayoutDelay(&delayMS) == -1) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice, VoEId(_instanceId, _channelId),
                 "Channel::GetPlayoutTimeStamp() failed to read playout"
                 " delay from the ADM");
    return -1;
  }

  WebRtc_Word32 playoutFrequency = _audioCodingModule.PlayoutFrequency();

  if (_audioCodingModule.ReceiveCodec(currRecCodec) == 0) {
    if (STR_CASE_CMP("G722", currRecCodec.plname) == 0) {
      playoutFrequency = 8000;
    } else if (STR_CASE_CMP("opus", currRecCodec.plname) == 0) {
      playoutFrequency = 48000;
    }
  }

  timestamp -= (delayMS * (playoutFrequency / 1000));

  playoutTimestamp = timestamp;

  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetPlayoutTimeStamp() => playoutTimestamp = %lu",
               playoutTimestamp);
  return 0;
}

void Channel::ResetDeadOrAliveCounters() {
  _countDeadDetections = 0;
  _countAliveDetections = 0;
}

void Channel::UpdateDeadOrAliveCounters(bool alive) {
  if (alive)
    _countAliveDetections++;
  else
    _countDeadDetections++;
}

int Channel::GetDeadOrAliveCounters(int& countDead, int& countAlive) const {
  bool enabled;
  WebRtc_UWord8 timeSec;

  _rtpRtcpModule->PeriodicDeadOrAliveStatus(enabled, timeSec);

  if (!enabled)
    return (-1);

  countDead = static_cast<int> (_countDeadDetections);
  countAlive = static_cast<int> (_countAliveDetections);
  return 0;
}

int Channel::StartPlayingFileLocally(const char* fileName,
                                     const bool loop,
                                     const FileFormats format,
                                     const int startPosition,
                                     const float volumeScaling,
                                     const int stopPosition,
                                     const CodecInst* codecInst) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StartPlayingFileLocally(fileNameUTF8[]=%s, loop=%d,"
               " format=%d, volumeScaling=%5.3f, startPosition=%d, "
               "stopPosition=%d)", fileName, loop, format, volumeScaling,
               startPosition, stopPosition);

  if (_outputFilePlaying) {
    _engineStatisticsPtr->SetLastError(
      VE_ALREADY_PLAYING, kTraceError,
      "StartPlayingFileLocally() is already playing");
    return -1;
  }

  {
    CriticalSectionScoped cs(&_fileCritSect);

    if (_outputFilePlayerPtr) {
      _outputFilePlayerPtr->RegisterModuleFileCallback(NULL);
      FilePlayer::DestroyFilePlayer(_outputFilePlayerPtr);
      _outputFilePlayerPtr = NULL;
    }

    _outputFilePlayerPtr = FilePlayer::CreateFilePlayer(
                             _outputFilePlayerId, (const FileFormats)format);

    if (_outputFilePlayerPtr == NULL) {
      _engineStatisticsPtr->SetLastError(
        VE_INVALID_ARGUMENT, kTraceError,
        "StartPlayingFileLocally() filePlayer format is not correct");
      return -1;
    }

    const WebRtc_UWord32 notificationTime(0);

    if (_outputFilePlayerPtr->StartPlayingFile(
          fileName,
          loop,
          startPosition,
          volumeScaling,
          notificationTime,
          stopPosition,
          (const CodecInst*)codecInst) != 0) {
      _engineStatisticsPtr->SetLastError(
        VE_BAD_FILE, kTraceError,
        "StartPlayingFile() failed to start file playout");
      _outputFilePlayerPtr->StopPlayingFile();
      FilePlayer::DestroyFilePlayer(_outputFilePlayerPtr);
      _outputFilePlayerPtr = NULL;
      return -1;
    }

    _outputFilePlayerPtr->RegisterModuleFileCallback(this);
    _outputFilePlaying = true;
  }

  if (RegisterFilePlayingToMixer() != 0)
    return -1;

  return 0;
}

int Channel::StopPlayingFileLocally() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StopPlayingFileLocally()");

  if (!_outputFilePlaying) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_OPERATION, kTraceWarning,
      "StopPlayingFileLocally() isnot playing");
    return 0;
  }

  {
    CriticalSectionScoped cs(&_fileCritSect);

    if (_outputFilePlayerPtr->StopPlayingFile() != 0) {
      _engineStatisticsPtr->SetLastError(
        VE_STOP_RECORDING_FAILED, kTraceError,
        "StopPlayingFile() could not stop playing");
      return -1;
    }

    _outputFilePlayerPtr->RegisterModuleFileCallback(NULL);
    FilePlayer::DestroyFilePlayer(_outputFilePlayerPtr);
    _outputFilePlayerPtr = NULL;
    _outputFilePlaying = false;
  }

  // _fileCritSect cannot be taken while calling
  // SetAnonymousMixibilityStatus. Refer to comments in
  // StartPlayingFileLocally(const char* ...) for more details.
  if (_outputMixerPtr->SetAnonymousMixabilityStatus(*this, false) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CONF_MIX_MODULE_ERROR, kTraceError,
      "StopPlayingFile() failed to stop participant from playing as"
      "file in the mixer");
    return -1;
  }

  return 0;
}

int Channel::IsPlayingFileLocally() const {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::IsPlayingFileLocally()");

  return (WebRtc_Word32)_outputFilePlaying;
}

int Channel::RegisterFilePlayingToMixer() {
  // Return success for not registering for file playing to mixer if:
  // 1. playing file before playout is started on that channel.
  // 2. starting playout without file playing on that channel.
  if (!_playing || !_outputFilePlaying) {
    return 0;
  }

  // |_fileCritSect| cannot be taken while calling
  // SetAnonymousMixabilityStatus() since as soon as the participant is added
  // frames can be pulled by the mixer. Since the frames are generated from
  // the file, _fileCritSect will be taken. This would result in a deadlock.
  if (_outputMixerPtr->SetAnonymousMixabilityStatus(*this, true) != 0) {
    CriticalSectionScoped cs(&_fileCritSect);
    _outputFilePlaying = false;
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CONF_MIX_MODULE_ERROR, kTraceError,
      "StartPlayingFile() failed to add participant as file to mixer");
    _outputFilePlayerPtr->StopPlayingFile();
    FilePlayer::DestroyFilePlayer(_outputFilePlayerPtr);
    _outputFilePlayerPtr = NULL;
    return -1;
  }

  return 0;
}

int Channel::ScaleLocalFilePlayout(const float scale) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::ScaleLocalFilePlayout(scale=%5.3f)", scale);

  CriticalSectionScoped cs(&_fileCritSect);

  if (!_outputFilePlaying) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_OPERATION, kTraceError,
      "ScaleLocalFilePlayout() isnot playing");
    return -1;
  }

  if ((_outputFilePlayerPtr == NULL) ||
      (_outputFilePlayerPtr->SetAudioScaling(scale) != 0)) {
    _engineStatisticsPtr->SetLastError(
      VE_BAD_ARGUMENT, kTraceError,
      "SetAudioScaling() failed to scale the playout");
    return -1;
  }

  return 0;
}

int Channel::GetLocalPlayoutPosition(int& positionMs) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetLocalPlayoutPosition(position=?)");

  WebRtc_UWord32 position;

  CriticalSectionScoped cs(&_fileCritSect);

  if (_outputFilePlayerPtr == NULL) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_OPERATION, kTraceError,
      "GetLocalPlayoutPosition() filePlayer instance doesnot exist");
    return -1;
  }

  if (_outputFilePlayerPtr->GetPlayoutPosition(position) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_BAD_FILE, kTraceError,
      "GetLocalPlayoutPosition() failed");
    return -1;
  }

  positionMs = position;

  return 0;
}

int Channel::StartPlayingFileAsMicrophone(const char* fileName,
    const bool loop,
    const FileFormats format,
    const int startPosition,
    const float volumeScaling,
    const int stopPosition,
    const CodecInst* codecInst) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StartPlayingFileAsMicrophone(fileNameUTF8[]=%s, "
               "loop=%d, format=%d, volumeScaling=%5.3f, startPosition=%d, "
               "stopPosition=%d)", fileName, loop, format, volumeScaling,
               startPosition, stopPosition);

  if (_inputFilePlaying) {
    _engineStatisticsPtr->SetLastError(
      VE_ALREADY_PLAYING, kTraceWarning,
      "StartPlayingFileAsMicrophone() filePlayer is playing");
    return 0;
  }

  CriticalSectionScoped cs(&_fileCritSect);

  // Destroy the old instance
  if (_inputFilePlayerPtr) {
    _inputFilePlayerPtr->RegisterModuleFileCallback(NULL);
    FilePlayer::DestroyFilePlayer(_inputFilePlayerPtr);
    _inputFilePlayerPtr = NULL;
  }

  // Create the instance
  _inputFilePlayerPtr = FilePlayer::CreateFilePlayer(
                          _inputFilePlayerId, (const FileFormats)format);

  if (_inputFilePlayerPtr == NULL) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_ARGUMENT, kTraceError,
      "StartPlayingFileAsMicrophone() filePlayer format isnot correct");
    return -1;
  }

  const WebRtc_UWord32 notificationTime(0);

  if (_inputFilePlayerPtr->StartPlayingFile(
        fileName,
        loop,
        startPosition,
        volumeScaling,
        notificationTime,
        stopPosition,
        (const CodecInst*)codecInst) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_BAD_FILE, kTraceError,
      "StartPlayingFile() failed to start file playout");
    _inputFilePlayerPtr->StopPlayingFile();
    FilePlayer::DestroyFilePlayer(_inputFilePlayerPtr);
    _inputFilePlayerPtr = NULL;
    return -1;
  }

  _inputFilePlayerPtr->RegisterModuleFileCallback(this);
  _inputFilePlaying = true;

  return 0;
}

int Channel::StopPlayingFileAsMicrophone() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StopPlayingFileAsMicrophone()");

  if (!_inputFilePlaying) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_OPERATION, kTraceWarning,
      "StopPlayingFileAsMicrophone() is not playing");
    return 0;
  }

  CriticalSectionScoped cs(&_fileCritSect);

  if (_inputFilePlayerPtr->StopPlayingFile() != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_STOP_RECORDING_FAILED, kTraceError,
      "StopPlayingFile() could not stop playing");
    _inputFilePlayerPtr->RegisterModuleFileCallback(NULL);
    FilePlayer::DestroyFilePlayer(_inputFilePlayerPtr);
    _inputFilePlayerPtr = NULL;
    _inputFilePlaying = false;
    return -1;
  }

  _inputFilePlayerPtr->RegisterModuleFileCallback(NULL);
  FilePlayer::DestroyFilePlayer(_inputFilePlayerPtr);
  _inputFilePlayerPtr = NULL;
  _inputFilePlaying = false;

  return 0;
}

int Channel::IsPlayingFileAsMicrophone() const {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::IsPlayingFileAsMicrophone()");

  return _inputFilePlaying;
}

int Channel::ScaleFileAsMicrophonePlayout(const float scale) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::ScaleFileAsMicrophonePlayout(scale=%5.3f)", scale);

  CriticalSectionScoped cs(&_fileCritSect);

  if (!_inputFilePlaying) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_OPERATION, kTraceError,
      "ScaleFileAsMicrophonePlayout() isnot playing");
    return -1;
  }

  if ((_inputFilePlayerPtr == NULL) ||
      (_inputFilePlayerPtr->SetAudioScaling(scale) != 0)) {
    _engineStatisticsPtr->SetLastError(
      VE_BAD_ARGUMENT, kTraceError,
      "SetAudioScaling() failed to scale playout");
    return -1;
  }

  return 0;
}


int Channel::StartRecordingPlayout(const char* fileName,
                                   const CodecInst* codecInst) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StartRecordingPlayout(fileName=%s)", fileName);

  if (_outputFileRecording) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice, VoEId(_instanceId, -1),
                 "StartRecordingPlayout() is already recording");
    return 0;
  }

  FileFormats format;
  const WebRtc_UWord32 notificationTime(0); // Not supported in VoE
  CodecInst dummyCodec = {100, "L16", 16000, 320, 1, 320000};

  if ((codecInst != NULL) &&
      ((codecInst->channels < 1) || (codecInst->channels > 2))) {
    _engineStatisticsPtr->SetLastError(
      VE_BAD_ARGUMENT, kTraceError,
      "StartRecordingPlayout() invalid compression");
    return(-1);
  }

  if(codecInst == NULL) {
    format = kFileFormatPcm16kHzFile;
    codecInst = &dummyCodec;
  } else if((STR_CASE_CMP(codecInst->plname, "L16") == 0) ||
            (STR_CASE_CMP(codecInst->plname, "PCMU") == 0) ||
            (STR_CASE_CMP(codecInst->plname, "PCMA") == 0)) {
    format = kFileFormatWavFile;
  } else {
    format = kFileFormatCompressedFile;
  }

  CriticalSectionScoped cs(&_fileCritSect);

  // Destroy the old instance
  if (_outputFileRecorderPtr) {
    _outputFileRecorderPtr->RegisterModuleFileCallback(NULL);
    FileRecorder::DestroyFileRecorder(_outputFileRecorderPtr);
    _outputFileRecorderPtr = NULL;
  }

  _outputFileRecorderPtr = FileRecorder::CreateFileRecorder(
                             _outputFileRecorderId, (const FileFormats)format);

  if (_outputFileRecorderPtr == NULL) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_ARGUMENT, kTraceError,
      "StartRecordingPlayout() fileRecorder format isnot correct");
    return -1;
  }

  if (_outputFileRecorderPtr->StartRecordingAudioFile(
        fileName, (const CodecInst&)*codecInst, notificationTime) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_BAD_FILE, kTraceError,
      "StartRecordingAudioFile() failed to start file recording");
    _outputFileRecorderPtr->StopRecording();
    FileRecorder::DestroyFileRecorder(_outputFileRecorderPtr);
    _outputFileRecorderPtr = NULL;
    return -1;
  }

  _outputFileRecorderPtr->RegisterModuleFileCallback(this);
  _outputFileRecording = true;

  return 0;
}

int Channel::StopRecordingPlayout() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, -1),
               "Channel::StopRecordingPlayout()");

  if (!_outputFileRecording) {
    WEBRTC_TRACE(kTraceError, kTraceVoice, VoEId(_instanceId, -1),
                 "StopRecordingPlayout() isnot recording");
    return -1;
  }


  CriticalSectionScoped cs(&_fileCritSect);

  if (_outputFileRecorderPtr->StopRecording() != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_STOP_RECORDING_FAILED, kTraceError,
      "StopRecording() could not stop recording");
    return(-1);
  }

  _outputFileRecorderPtr->RegisterModuleFileCallback(NULL);
  FileRecorder::DestroyFileRecorder(_outputFileRecorderPtr);
  _outputFileRecorderPtr = NULL;
  _outputFileRecording = false;

  return 0;
}

#ifdef WEBRTC_VOICE_ENGINE_NR

int Channel::SetRxNsStatus(const bool enable, const NsModes mode) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetRxNsStatus(enable=%d, mode=%d)",
               (int)enable, (int)mode);

  NoiseSuppression::Level nsLevel(
    (NoiseSuppression::Level)WEBRTC_VOICE_ENGINE_RX_NS_DEFAULT_MODE);

  switch (mode) {
    case kNsDefault:
      nsLevel = (NoiseSuppression::Level)
                WEBRTC_VOICE_ENGINE_RX_NS_DEFAULT_MODE;
      break;

    case kNsUnchanged:
      nsLevel = _rxAudioProcessingModulePtr->noise_suppression()->level();
      break;

    case kNsConference:
      nsLevel = NoiseSuppression::kHigh;
      break;

    case kNsLowSuppression:
      nsLevel = NoiseSuppression::kLow;
      break;

    case kNsModerateSuppression:
      nsLevel = NoiseSuppression::kModerate;
      break;

    case kNsHighSuppression:
      nsLevel = NoiseSuppression::kHigh;
      break;

    case kNsVeryHighSuppression:
      nsLevel = NoiseSuppression::kVeryHigh;
      break;
  }

  if (_rxAudioProcessingModulePtr->noise_suppression()->set_level(nsLevel)
      != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceError,
      "SetRxAgcStatus() failed to set Ns level");
    return -1;
  }

  if (_rxAudioProcessingModulePtr->noise_suppression()->Enable(enable) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceError,
      "SetRxAgcStatus() failed to set Agc state");
    return -1;
  }

  _rxNsIsEnabled = enable;
  _rxApmIsEnabled = ((_rxAgcIsEnabled == true) || (_rxNsIsEnabled == true));

  return 0;
}

int Channel::GetRxNsStatus(bool& enabled, NsModes& mode) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetRxNsStatus(enable=?, mode=?)");

  bool enable =
    _rxAudioProcessingModulePtr->noise_suppression()->is_enabled();
  NoiseSuppression::Level ncLevel =
    _rxAudioProcessingModulePtr->noise_suppression()->level();

  enabled = enable;

  switch (ncLevel) {
    case NoiseSuppression::kLow:
      mode = kNsLowSuppression;
      break;

    case NoiseSuppression::kModerate:
      mode = kNsModerateSuppression;
      break;

    case NoiseSuppression::kHigh:
      mode = kNsHighSuppression;
      break;

    case NoiseSuppression::kVeryHigh:
      mode = kNsVeryHighSuppression;
      break;
  }

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetRxNsStatus() => enabled=%d, mode=%d", enabled, mode);
  return 0;
}

#endif // #ifdef WEBRTC_VOICE_ENGINE_NR

#ifdef WEBRTC_VOICE_ENGINE_AGC

int Channel::SetRxAgcStatus(const bool enable, const AgcModes mode) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetRxAgcStatus(enable=%d, mode=%d)",
               (int)enable, (int)mode);

  GainControl::Mode agcMode(GainControl::kFixedDigital);

  switch (mode) {
    case kAgcDefault:
      agcMode = GainControl::kAdaptiveDigital;
      break;

    case kAgcUnchanged:
      agcMode = _rxAudioProcessingModulePtr->gain_control()->mode();
      break;

    case kAgcFixedDigital:
      agcMode = GainControl::kFixedDigital;
      break;

    case kAgcAdaptiveDigital:
      agcMode = GainControl::kAdaptiveDigital;
      break;

    default:
      _engineStatisticsPtr->SetLastError(
        VE_INVALID_ARGUMENT, kTraceError,
        "SetRxAgcStatus() invalid Agc mode");
      return -1;
  }

  if (_rxAudioProcessingModulePtr->gain_control()->set_mode(agcMode) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceError,
      "SetRxAgcStatus() failed to set Agc mode");
    return -1;
  }

  if (_rxAudioProcessingModulePtr->gain_control()->Enable(enable) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceError,
      "SetRxAgcStatus() failed to set Agc state");
    return -1;
  }

  _rxAgcIsEnabled = enable;
  _rxApmIsEnabled = ((_rxAgcIsEnabled == true) || (_rxNsIsEnabled == true));

  return 0;
}

int Channel::GetRxAgcStatus(bool& enabled, AgcModes& mode) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetRxAgcStatus(enable=?, mode=?)");

  bool enable = _rxAudioProcessingModulePtr->gain_control()->is_enabled();
  GainControl::Mode agcMode =
    _rxAudioProcessingModulePtr->gain_control()->mode();

  enabled = enable;

  switch (agcMode) {
    case GainControl::kFixedDigital:
      mode = kAgcFixedDigital;
      break;

    case GainControl::kAdaptiveDigital:
      mode = kAgcAdaptiveDigital;
      break;

    default:
      _engineStatisticsPtr->SetLastError(
        VE_APM_ERROR, kTraceError,
        "GetRxAgcStatus() invalid Agc mode");
      return -1;
  }

  return 0;
}

int Channel::SetRxAgcConfig(const AgcConfig config) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetRxAgcConfig()");

  if (_rxAudioProcessingModulePtr->gain_control()->set_target_level_dbfs(
        config.targetLeveldBOv) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceError,
      "SetRxAgcConfig() failed to set target peak |level|"
      "(or envelope) of the Agc");
    return -1;
  }

  if (_rxAudioProcessingModulePtr->gain_control()->set_compression_gain_db(
        config.digitalCompressionGaindB) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceError,
      "SetRxAgcConfig() failed to set the range in |gain| the"
      " digital compression stage may apply");
    return -1;
  }

  if (_rxAudioProcessingModulePtr->gain_control()->enable_limiter(
        config.limiterEnable) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_APM_ERROR, kTraceError,
      "SetRxAgcConfig() failed to set hard limiter to the signal");
    return -1;
  }

  return 0;
}

int Channel::GetRxAgcConfig(AgcConfig& config) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetRxAgcConfig(config=%?)");

  config.targetLeveldBOv =
    _rxAudioProcessingModulePtr->gain_control()->target_level_dbfs();
  config.digitalCompressionGaindB =
    _rxAudioProcessingModulePtr->gain_control()->compression_gain_db();
  config.limiterEnable =
    _rxAudioProcessingModulePtr->gain_control()->is_limiter_enabled();

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId), "GetRxAgcConfig() => "
               "targetLeveldBOv=%u, digitalCompressionGaindB=%u,"
               " limiterEnable=%d",
               config.targetLeveldBOv,
               config.digitalCompressionGaindB,
               config.limiterEnable);

  return 0;
}

#endif // #ifdef WEBRTC_VOICE_ENGINE_AGC

WebRtc_Word32 Channel::SetBackgroundNoiseMode(const ACMBackgroundNoiseMode
    mode) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetBackgroundNoiseMode()");
  return _audioCodingModule.SetBackgroundNoiseMode(mode);
}

WebRtc_Word32 Channel::BackgroundNoiseMode(ACMBackgroundNoiseMode& mode) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::BackgroundNoiseMode()");
  return _audioCodingModule.BackgroundNoiseMode(mode);
}

void Channel::SetMixWithMicStatus(bool mix) {
  _mixFileWithMicrophone = mix;
}

int Channel::GetSpeechOutputLevel(WebRtc_UWord32& level) const {
  WebRtc_Word8 currentLevel = _outputAudioLevel.Level();
  level = static_cast<WebRtc_Word32> (currentLevel);
  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetSpeechOutputLevel() => level=%u", level);
  return 0;
}

int Channel::GetSpeechOutputLevelFullRange(WebRtc_UWord32& level) const {
  WebRtc_Word16 currentLevel = _outputAudioLevel.LevelFullRange();
  level = static_cast<WebRtc_Word32> (currentLevel);
  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetSpeechOutputLevelFullRange() => level=%u", level);
  return 0;
}

int Channel::SetMute(bool enable) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetMute(enable=%d)", enable);
  _mute = enable;
  return 0;
}

bool Channel::Mute() const {
  return _mute;
}

int Channel::SetOutputVolumePan(float left, float right) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetOutputVolumePan()");
  _panLeft = left;
  _panRight = right;
  return 0;
}

int Channel::GetOutputVolumePan(float& left, float& right) const {
  left = _panLeft;
  right = _panRight;
  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetOutputVolumePan() => left=%3.2f, right=%3.2f", left, right);
  return 0;
}

int Channel::SetChannelOutputVolumeScaling(float scaling) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetChannelOutputVolumeScaling()");
  _outputGain = scaling;
  return 0;
}

int Channel::GetChannelOutputVolumeScaling(float& scaling) const {
  scaling = _outputGain;
  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetChannelOutputVolumeScaling() => scaling=%3.2f", scaling);
  return 0;
}

WebRtc_UWord32 Channel::Demultiplex(const AudioFrame& audioFrame) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::Demultiplex()");
  _audioFrame = audioFrame;
  _audioFrame.id_ = _channelId;
  return 0;
}

WebRtc_UWord32 Channel::PrepareEncodeAndSend(int mixingFrequency) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::PrepareEncodeAndSend()");

  if (_audioFrame.samples_per_channel_ == 0) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice, VoEId(_instanceId, _channelId),
                 "Channel::PrepareEncodeAndSend() invalid audio frame");
    return -1;
  }

  if (_inputFilePlaying) {
    MixOrReplaceAudioWithFile(mixingFrequency);
  }

  if (_mute) {
    AudioFrameOperations::Mute(_audioFrame);
  }

  if (_inputExternalMedia) {
    CriticalSectionScoped cs(&_callbackCritSect);
    const bool isStereo = (_audioFrame.num_channels_ == 2);

    if (_inputExternalMediaCallbackPtr) {
      _inputExternalMediaCallbackPtr->Process(
        _channelId,
        kRecordingPerChannel,
        (WebRtc_Word16*)_audioFrame.data_,
        _audioFrame.samples_per_channel_,
        _audioFrame.sample_rate_hz_,
        isStereo);
    }
  }

  //InsertInbandDtmfTone();

  if (_includeAudioLevelIndication) {
    assert(_rtpAudioProc.get() != NULL);

    // Check if settings need to be updated.
    if (_rtpAudioProc->sample_rate_hz() != _audioFrame.sample_rate_hz_) {
      if (_rtpAudioProc->set_sample_rate_hz(_audioFrame.sample_rate_hz_) !=
          AudioProcessing::kNoError) {
        WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                     VoEId(_instanceId, _channelId),
                     "Error setting AudioProcessing sample rate");
        return -1;
      }
    }

    if (_rtpAudioProc->num_input_channels() != _audioFrame.num_channels_) {
      if (_rtpAudioProc->set_num_channels(_audioFrame.num_channels_,
                                          _audioFrame.num_channels_)
          != AudioProcessing::kNoError) {
        WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                     VoEId(_instanceId, _channelId),
                     "Error setting AudioProcessing channels");
        return -1;
      }
    }

    // Performs level analysis only; does not affect the signal.
    _rtpAudioProc->ProcessStream(&_audioFrame);
  }

  return 0;
}

WebRtc_UWord32 Channel::EncodeAndSend() {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::EncodeAndSend()");

  assert(_audioFrame.num_channels_ <= 2);

  if (_audioFrame.samples_per_channel_ == 0) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice, VoEId(_instanceId, _channelId),
                 "Channel::EncodeAndSend() invalid audio frame");
    return -1;
  }

  _audioFrame.id_ = _channelId;

  // --- Add 10ms of raw (PCM) audio data to the encoder @ 32kHz.

  // The ACM resamples internally.
  _audioFrame.timestamp_ = _timeStamp;

  if (_audioCodingModule.Add10MsData((AudioFrame&)_audioFrame) != 0) {
    WEBRTC_TRACE(kTraceError, kTraceVoice, VoEId(_instanceId, _channelId),
                 "Channel::EncodeAndSend() ACM encoding failed");
    return -1;
  }

  _timeStamp += _audioFrame.samples_per_channel_;

  // --- Encode if complete frame is ready

  // This call will trigger AudioPacketizationCallback::SendData if encoding
  // is done and payload is ready for packetization and transmission.
  return _audioCodingModule.Process();
}

int Channel::RegisterExternalMediaProcessing(
  ProcessingTypes type,
  VoEMediaProcess& processObject) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::RegisterExternalMediaProcessing()");

  CriticalSectionScoped cs(&_callbackCritSect);

  /*if (kPlaybackPerChannel == type)
  {
    if (_outputExternalMediaCallbackPtr)
    {
      _engineStatisticsPtr->SetLastError(
        VE_INVALID_OPERATION, kTraceError,
        "Channel::RegisterExternalMediaProcessing() "
        "output external media already enabled");
      return -1;
    }
    _outputExternalMediaCallbackPtr = &processObject;
    _outputExternalMedia = true;
  }
  else if (kRecordingPerChannel == type)
  {
    if (_inputExternalMediaCallbackPtr)
    {
      _engineStatisticsPtr->SetLastError(
        VE_INVALID_OPERATION, kTraceError,
        "Channel::RegisterExternalMediaProcessing() "
        "output external media already enabled");
      return -1;
    }
    _inputExternalMediaCallbackPtr = &processObject;
    _inputExternalMedia = true;
  }*/
  return 0;
}

int Channel::DeRegisterExternalMediaProcessing(ProcessingTypes type) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::DeRegisterExternalMediaProcessing()");

  CriticalSectionScoped cs(&_callbackCritSect);

  /*if (kPlaybackPerChannel == type)
  {
    if (!_outputExternalMediaCallbackPtr)
    {
      _engineStatisticsPtr->SetLastError(
        VE_INVALID_OPERATION, kTraceWarning,
        "Channel::DeRegisterExternalMediaProcessing() "
        "output external media already disabled");
      return 0;
    }
    _outputExternalMedia = false;
    _outputExternalMediaCallbackPtr = NULL;
  }
  else if (kRecordingPerChannel == type)
  {
    if (!_inputExternalMediaCallbackPtr)
    {
      _engineStatisticsPtr->SetLastError(
        VE_INVALID_OPERATION, kTraceWarning,
        "Channel::DeRegisterExternalMediaProcessing() "
        "input external media already disabled");
      return 0;
    }
    _inputExternalMedia = false;
    _inputExternalMediaCallbackPtr = NULL;
  }*/

  return 0;
}

WebRtc_Word32 Channel::RegisterExternalTransport(Transport& transport) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::RegisterExternalTransport()");

  CriticalSectionScoped cs(&_callbackCritSect);

//#ifndef WEBRTC_EXTERNAL_TRANSPORT
//  // Sanity checks for default (non external transport) to avoid conflict with
//  // WebRtc sockets.
//  if (_socketTransportModule.SendSocketsInitialized())
//  {
//    _engineStatisticsPtr->SetLastError(VE_SEND_SOCKETS_CONFLICT,
//      kTraceError,
//      "RegisterExternalTransport() send sockets already initialized");
//    return -1;
//  }
//  if (_socketTransportModule.ReceiveSocketsInitialized())
//  {
//    _engineStatisticsPtr->SetLastError(VE_RECEIVE_SOCKETS_CONFLICT,
//      kTraceError,
//      "RegisterExternalTransport() receive sockets already initialized");
//    return -1;
//  }
//#endif
  if (_externalTransport) {
    _engineStatisticsPtr->SetLastError(VE_INVALID_OPERATION,
                                       kTraceError,
                                       "RegisterExternalTransport() external transport already enabled");
    return -1;
  }

  _externalTransport = true;
  _transportPtr = &transport;
  return 0;
}

WebRtc_Word32 Channel::DeRegisterExternalTransport() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::DeRegisterExternalTransport()");

  CriticalSectionScoped cs(&_callbackCritSect);

  if (!_transportPtr) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_OPERATION, kTraceWarning,
      "DeRegisterExternalTransport() external transport already "
      "disabled");
    return 0;
  }

  _externalTransport = false;

  _transportPtr = NULL;
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "DeRegisterExternalTransport() all transport is disabled");
  return 0;
}


WebRtc_Word32 Channel::ReceivedRTPPacket(const WebRtc_Word8* data,
    WebRtc_Word32 length) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::ReceivedRTPPacket()");

  if (_enablePacketSeparate) {
    if (_dataSeparatorPtr) {
      WebRtcRTPHeader rtp_hdr = {0};

      ModuleRTPUtility::RTPHeaderParser rtpParser((WebRtc_UWord8*)data, length);

      rtpParser.Parse(rtp_hdr);
      _dataSeparatorPtr->ParseSegmentPacket((WebRtc_UWord8*)data +
                                            rtp_hdr.header.headerLength, length, &rtp_hdr);
    }
  } else {
    const char dummyIP[] = "127.0.0.1";
    IncomingRTPPacket(data, length, dummyIP, 0);
  }

  return 0;
}

WebRtc_Word32 Channel::ReceivedRTCPPacket(const WebRtc_Word8* data,
    WebRtc_Word32 length) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::ReceivedRTCPPacket()");
  const char dummyIP[] = "127.0.0.1";
  IncomingRTCPPacket(data, length, dummyIP, 0);
  return 0;
}

int Channel::GetRTPStatistics(
  unsigned int& averageJitterMs,
  unsigned int& maxJitterMs,
  unsigned int& discardedPackets) {
  WebRtc_UWord8 fraction_lost(0);
  WebRtc_UWord32 cum_lost(0);
  WebRtc_UWord32 ext_max(0);
  WebRtc_UWord32 jitter(0);
  WebRtc_UWord32 max_jitter(0);

  // The jitter statistics is updated for each received RTP packet and is
  // based on received packets.
  if (_rtpRtcpModule->StatisticsRTP(&fraction_lost,
                                    &cum_lost,
                                    &ext_max,
                                    &jitter,
                                    &max_jitter) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_CANNOT_RETRIEVE_RTP_STAT, kTraceWarning,
      "GetRTPStatistics() failed to read RTP statistics from the "
      "RTP/RTCP module");
  }

  const WebRtc_Word32 playoutFrequency =
    _audioCodingModule.PlayoutFrequency();

  if (playoutFrequency > 0) {
    // Scale RTP statistics given the current playout frequency
    maxJitterMs = max_jitter / (playoutFrequency / 1000);
    averageJitterMs = jitter / (playoutFrequency / 1000);
  }

  discardedPackets = _numberOfDiscardedPackets;

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetRTPStatistics() => averageJitterMs = %lu, maxJitterMs = %lu,"
               " discardedPackets = %lu)",
               averageJitterMs, maxJitterMs, discardedPackets);
  return 0;
}

int Channel::GetRTPStatistics(CallStatistics& stats) {
  WebRtc_UWord8 fraction_lost(0);
  WebRtc_UWord32 cum_lost(0);
  WebRtc_UWord32 ext_max(0);
  WebRtc_UWord32 jitter(0);
  WebRtc_UWord32 max_jitter(0);

  // --- Part one of the final structure (four values)

  // The jitter statistics is updated for each received RTP packet and is
  // based on received packets.
  if (_rtpRtcpModule->StatisticsRTP(&fraction_lost,
                                    &cum_lost,
                                    &ext_max,
                                    &jitter,
                                    &max_jitter) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_CANNOT_RETRIEVE_RTP_STAT, kTraceWarning,
      "GetRTPStatistics() failed to read RTP statistics from the "
      "RTP/RTCP module");
  }

  stats.fractionLost = fraction_lost;
  stats.cumulativeLost = cum_lost;
  stats.extendedMax = ext_max;
  stats.jitterSamples = jitter;

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetRTPStatistics() => fractionLost=%lu, cumulativeLost=%lu,"
               " extendedMax=%lu, jitterSamples=%li)",
               stats.fractionLost, stats.cumulativeLost, stats.extendedMax,
               stats.jitterSamples);

  // --- Part two of the final structure (one value)

  WebRtc_UWord16 RTT(0);
  RTCPMethod method = _rtpRtcpModule->RTCP();

  if (method == kRtcpOff) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "GetRTPStatistics() RTCP is disabled => valid RTT "
                 "measurements cannot be retrieved");
  } else {
    // The remote SSRC will be zero if no RTP packet has been received.
    WebRtc_UWord32 remoteSSRC = _rtpRtcpModule->RemoteSSRC();

    if (remoteSSRC > 0) {
      WebRtc_UWord16 avgRTT(0);
      WebRtc_UWord16 maxRTT(0);
      WebRtc_UWord16 minRTT(0);

      if (_rtpRtcpModule->RTT(remoteSSRC, &RTT, &avgRTT, &minRTT, &maxRTT)
          != 0) {
        WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                     VoEId(_instanceId, _channelId),
                     "GetRTPStatistics() failed to retrieve RTT from "
                     "the RTP/RTCP module");
      }
    } else {
      WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                   VoEId(_instanceId, _channelId),
                   "GetRTPStatistics() failed to measure RTT since no "
                   "RTP packets have been received yet");
    }
  }

  stats.rttMs = static_cast<int> (RTT);

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetRTPStatistics() => rttMs=%d", stats.rttMs);

  // --- Part three of the final structure (four values)

  WebRtc_UWord32 bytesSent(0);
  WebRtc_UWord32 packetsSent(0);
  WebRtc_UWord32 bytesReceived(0);
  WebRtc_UWord32 packetsReceived(0);

  if (_rtpRtcpModule->DataCountersRTP(&bytesSent,
                                      &packetsSent,
                                      &bytesReceived,
                                      &packetsReceived) != 0) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_instanceId, _channelId),
                 "GetRTPStatistics() failed to retrieve RTP datacounters =>"
                 " output will not be complete");
  }

  stats.bytesSent = bytesSent;
  stats.packetsSent = packetsSent;
  stats.bytesReceived = bytesReceived;
  stats.packetsReceived = packetsReceived;

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetRTPStatistics() => bytesSent=%d, packetsSent=%d,"
               " bytesReceived=%d, packetsReceived=%d)",
               stats.bytesSent, stats.packetsSent, stats.bytesReceived,
               stats.packetsReceived);

  return 0;
}

int Channel::StartRTPDump(const char fileNameUTF8[1024],
                          RTPDirections direction) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StartRTPDump()");

  if ((direction != kRtpIncoming) && (direction != kRtpOutgoing)) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_ARGUMENT, kTraceError,
      "StartRTPDump() invalid RTP direction");
    return -1;
  }

  RtpDump* rtpDumpPtr = (direction == kRtpIncoming) ?
                        &_rtpDumpIn : &_rtpDumpOut;

  if (rtpDumpPtr == NULL) {
    assert(false);
    return -1;
  }

  if (rtpDumpPtr->IsActive()) {
    rtpDumpPtr->Stop();
  }

  if (rtpDumpPtr->Start(fileNameUTF8) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_BAD_FILE, kTraceError,
      "StartRTPDump() failed to create file");
    return -1;
  }

  return 0;
}

int Channel::StopRTPDump(RTPDirections direction) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::StopRTPDump()");

  if ((direction != kRtpIncoming) && (direction != kRtpOutgoing)) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_ARGUMENT, kTraceError,
      "StopRTPDump() invalid RTP direction");
    return -1;
  }

  RtpDump* rtpDumpPtr = (direction == kRtpIncoming) ?
                        &_rtpDumpIn : &_rtpDumpOut;

  if (rtpDumpPtr == NULL) {
    assert(false);
    return -1;
  }

  if (!rtpDumpPtr->IsActive()) {
    return 0;
  }

  return rtpDumpPtr->Stop();
}

bool Channel::RTPDumpIsActive(RTPDirections direction) {
  if ((direction != kRtpIncoming) && (direction != kRtpOutgoing)) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_ARGUMENT, kTraceError,
      "RTPDumpIsActive() invalid RTP direction");
    return false;
  }

  RtpDump* rtpDumpPtr = (direction == kRtpIncoming) ?
                        &_rtpDumpIn : &_rtpDumpOut;
  return rtpDumpPtr->IsActive();
}

int Channel::GetNetworkStatistics(NetworkStatistics& stats) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetNetworkStatistics()");
  return _audioCodingModule.NetworkStatistics(
           (ACMNetworkStatistics &)stats);
}

int Channel::GetDelayEstimate(int& delayMs) const {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetDelayEstimate()");
  delayMs = (_averageDelayMs + 5) / 10 + _recPacketDelayMs;
  return 0;
}

int Channel::SetMinimumPlayoutDelay(int delayMs) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetMinimumPlayoutDelay()");

  if ((delayMs < kVoiceEngineMinMinPlayoutDelayMs) ||
      (delayMs > kVoiceEngineMaxMinPlayoutDelayMs)) {
    _engineStatisticsPtr->SetLastError(
      VE_INVALID_ARGUMENT, kTraceError,
      "SetMinimumPlayoutDelay() invalid min delay");
    return -1;
  }

  if (_audioCodingModule.SetMinimumPlayoutDelay(delayMs) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_AUDIO_CODING_MODULE_ERROR, kTraceError,
      "SetMinimumPlayoutDelay() failed to set min playout delay");
    return -1;
  }

  return 0;
}

int Channel::GetPlayoutTimestamp(unsigned int& timestamp) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetPlayoutTimestamp()");
  WebRtc_UWord32 playoutTimestamp(0);

  if (GetPlayoutTimeStamp(playoutTimestamp) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_CANNOT_RETRIEVE_VALUE, kTraceError,
      "GetPlayoutTimestamp() failed to retrieve timestamp");
    return -1;
  }

  timestamp = playoutTimestamp;
  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_instanceId, _channelId),
               "GetPlayoutTimestamp() => timestamp=%u", timestamp);
  return 0;
}

int Channel::GetDelayWithPlayoutTimeStamp(unsigned int timestamp,
    unsigned int &delay) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::GetDelayWithPlayoutTimeStamp()");

  if (_cntPerMs <= 0) return 0;

  WebRtc_UWord32 playoutTimestamp(0);

  if (GetPlayoutTimeStamp(playoutTimestamp) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_CANNOT_RETRIEVE_VALUE, kTraceError,
      "GetPlayoutTimestamp() failed to retrieve timestamp");
    return -1;
  }

  delay = timestamp - playoutTimestamp;
  delay /= _cntPerMs;

  return 0;
}

int Channel::SetInitTimestamp(unsigned int timestamp) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetInitTimestamp()");

  if (_sending) {
    _engineStatisticsPtr->SetLastError(
      VE_SENDING, kTraceError, "SetInitTimestamp() already sending");
    return -1;
  }

  if (_rtpRtcpModule->SetStartTimestamp(timestamp) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_RTP_RTCP_MODULE_ERROR, kTraceError,
      "SetInitTimestamp() failed to set timestamp");
    return -1;
  }

  return 0;
}

int Channel::SetInitSequenceNumber(short sequenceNumber) {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::SetInitSequenceNumber()");

  if (_sending) {
    _engineStatisticsPtr->SetLastError(
      VE_SENDING, kTraceError,
      "SetInitSequenceNumber() already sending");
    return -1;
  }

  if (_rtpRtcpModule->SetSequenceNumber(sequenceNumber) != 0) {
    _engineStatisticsPtr->SetLastError(
      VE_RTP_RTCP_MODULE_ERROR, kTraceError,
      "SetInitSequenceNumber() failed to set sequence number");
    return -1;
  }

  return 0;
}

int Channel::UpdateRxVadDetection(AudioFrame& audioFrame) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::UpdateRxVadDetection()");

  int vadDecision = 1;

  vadDecision = (audioFrame.vad_activity_ == AudioFrame::kVadActive) ? 1 : 0;

  if ((vadDecision != _oldVadDecision) /*&& _rxVadObserverPtr*/) {
    OnRxVadDetected(vadDecision);
    _oldVadDecision = vadDecision;
  }

  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_instanceId, _channelId),
               "Channel::UpdateRxVadDetection() => vadDecision=%d",
               vadDecision);
  return 0;
}

int Channel::ApmProcessRx(AudioFrame& frame) {
  AudioProcessing* audioproc = _rxAudioProcessingModulePtr;

  // Register the (possibly new) frame parameters.
  if (audioproc->set_sample_rate_hz(frame.sample_rate_hz_) != 0) {
    LOG_FERR1(LS_WARNING, set_sample_rate_hz, frame.sample_rate_hz_);
  }

  if (audioproc->set_num_channels(frame.num_channels_,
                                  frame.num_channels_) != 0) {
    LOG_FERR1(LS_WARNING, set_num_channels, frame.num_channels_);
  }

  if (audioproc->ProcessStream(&frame) != 0) {
    LOG_FERR0(LS_WARNING, ProcessStream);
  }

  return 0;
}


}  // namespace voe
}  // namespace webrtc