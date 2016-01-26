#include "engine_network_impl.h"
#include "trace.h"
#include "voe_errors.h"
#include "channel.h"
#include "shared_data.h"
#include "engine_network.h"
#include "rtp_utility.h"

using namespace webrtc;

int MediaEngineExternalTransport::SendPacket(int channel, const void *data,
    int len) {
  if (_transport) {
    ModuleRTPUtility::RTPHeaderParser rtpParser((WebRtc_UWord8*)data, len);
    WebRtcRTPHeader rtp_header;
    rtpParser.Parse(rtp_header);

    if (rtpParser.RTCP()) return 0;

    const char* raw_data = (const char*)data + rtp_header.header.headerLength;
    int raw_len = len - rtp_header.header.headerLength;

    int ret = _transport->outgo_audio_data(channel, rtp_header.header.markerBit,
                                           rtp_header.header.payloadType, rtp_header.header.sequenceNumber,
                                           rtp_header.header.timestamp, rtp_header.header.ssrc,
                                           (const char*)raw_data, raw_len);

    if (ret) ret += rtp_header.header.headerLength;

    return ret;
  } else {
    return 0;
  }
}

int MediaEngineExternalTransport::SendRTCPPacket(int channel, const void *data,
    int len) {
  // currently we don't need to send RTCP data
  return 0;
}

MediaEngineExternalTransport::MediaEngineExternalTransport(
  engine_transport *transport)
  : _transport(transport) {
}

MediaEngineNetworkImpl::MediaEngineNetworkImpl(SharedData* shared) : _shared(
    shared) {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "VoENetworkImpl() - ctor");
}

MediaEngineNetworkImpl::~MediaEngineNetworkImpl() {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "~VoENetworkImpl() - dtor");
}

int MediaEngineNetworkImpl::RegisterTransport(engine_transport *transport) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "RegisterTransport");
  _shared->set_external_transport(transport);
  return 0;
}

WebRtc_Word32 BuildRTPheader(WebRtc_UWord8* dataBuffer,
                             const WebRtc_Word8 payloadType,
                             const WebRtc_Word16 sequenceNumber,
                             const bool markerBit,
                             const WebRtc_UWord32 captureTimeStamp,
                             const WebRtc_UWord32 ssrc) {
  assert(payloadType >= 0);


  dataBuffer[0] = static_cast<WebRtc_UWord8>(0x80);  // version 2
  dataBuffer[1] = static_cast<WebRtc_UWord8>(payloadType);

  if (markerBit) {
    dataBuffer[1] |= kRtpMarkerBitMask;  // MarkerBit is set
  }

  ModuleRTPUtility::AssignUWord16ToBuffer(dataBuffer + 2, sequenceNumber);
  ModuleRTPUtility::AssignUWord32ToBuffer(dataBuffer + 4, captureTimeStamp);
  ModuleRTPUtility::AssignUWord32ToBuffer(dataBuffer + 8, ssrc);
  WebRtc_Word32 rtpHeaderLength = 12;

  return rtpHeaderLength;
}

int MediaEngineNetworkImpl::ReceivedRawPacket(int channel, uint8_t bit_mask,
    uint8_t pt, uint16_t seq, uint32_t tm, uint32_t ssrc, const char * data,
    int length) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "ReceivedRawPacket(channel=%d, length=%u)", channel, length);
  WebRtc_UWord8 buf[IP_PACKET_SIZE - 28];
  uint16_t max_packet_size = IP_PACKET_SIZE - 28;
  int hdr_len = BuildRTPheader(buf, pt, seq, bit_mask > 0, tm, ssrc);
  int data_len = max_packet_size - hdr_len;

  if (data_len < length) {
    _shared->SetLastError(VE_INVALID_PACKET, kTraceError,
                          "ReceivedRawPacket() invalid packet length");
    return -1;
  }

  memcpy(buf + hdr_len, data, length);
  data_len = length + hdr_len;
  return ReceivedRTPPacket(channel, buf, data_len);
}

int MediaEngineNetworkImpl::ReceivedRTPPacket(int channel,
    const void* data, unsigned int length) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "ReceivedRTPPacket(channel=%d, length=%u)", channel, length);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  if ((length < 12) || (length > 807)) {
    _shared->SetLastError(VE_INVALID_PACKET, kTraceError,
                          "ReceivedRTPPacket() invalid packet length");
    return -1;
  }

  if (NULL == data) {
    _shared->SetLastError(VE_INVALID_ARGUMENT, kTraceError,
                          "ReceivedRTPPacket() invalid data vector");
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "ReceivedRTPPacket() failed to locate channel");
    return -1;
  }

  if (!channelPtr->ExternalTransport()) {
    _shared->SetLastError(VE_INVALID_OPERATION, kTraceError,
                          "ReceivedRTPPacket() external transport is not enabled");
    return -1;
  }

  return channelPtr->ReceivedRTPPacket((const WebRtc_Word8*) data, length);
}

int MediaEngineNetworkImpl::ReceivedRTCPPacket(int channel, const void* data,
    unsigned int length) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "ReceivedRTCPPacket(channel=%d, length=%u)", channel, length);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  if (length < 4) {
    _shared->SetLastError(VE_INVALID_PACKET, kTraceError,
                          "ReceivedRTCPPacket() invalid packet length");
    return -1;
  }

  if (NULL == data) {
    _shared->SetLastError(VE_INVALID_ARGUMENT, kTraceError,
                          "ReceivedRTCPPacket() invalid data vector");
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "ReceivedRTCPPacket() failed to locate channel");
    return -1;
  }

  if (!channelPtr->ExternalTransport()) {
    _shared->SetLastError(VE_INVALID_OPERATION, kTraceError,
                          "ReceivedRTCPPacket() external transport is not enabled");
    return -1;
  }

  return channelPtr->ReceivedRTCPPacket((const WebRtc_Word8*) data, length);
}