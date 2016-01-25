#include "audio_data_combiner.h"
#include "modules/rtp_rtcp/source/rtp_sender.h"

namespace webrtc {

AudioDataCombiner::AudioDataCombiner(AudioDataCombinerCallback *callback)
  : _enable(true), _latest_pt(0), _latest_seq(0), _begin_seq(0),
    _max_data_size(IP_PACKET_SIZE - 28 - 12), _buffered_packet(0),
    _max_buffered_packet(5), _wr_pos(0), _cng_NB_pt(-1), _cng_WB_pt(-1),
    _cng_SWB_pt(-1), _cng_FB_pt(-1), _inband_VAD_active(false),
    _begin_timestamp(0), _new_segment(true), _data_callback(callback),
    _startTimeStamp(0), _ssrc(0), _sequenceNumber(0), _timeStamp(0) {
}

AudioDataCombiner::~AudioDataCombiner() {
}

void AudioDataCombiner::RegisterCNPaylodType(uint8_t nb_pt, uint8_t wb_pt,
    uint8_t swb_pt, uint8_t fb_pt) {
  _cng_NB_pt = nb_pt;
  _cng_WB_pt = wb_pt;
  _cng_SWB_pt = swb_pt;
  _cng_FB_pt = fb_pt;
}

void AudioDataCombiner::SetMaxBufferedPacket(uint16_t max_packet) {
  assert(max_packet > 0);
  _max_buffered_packet = max_packet;
}

bool AudioDataCombiner::MarkerBit(const WebRtc_Word8 payloadType) {
  // for audio true for first packet in a speech burst
  bool markerBit = false;

  if(_latest_pt != payloadType) {
    if(_cng_NB_pt != -1) {
      // we have configured NB CNG
      if(_cng_NB_pt == payloadType) {
        // only set a marker bit when we change payload type to a non CNG
        return false;
      }
    }

    if(_cng_WB_pt != -1) {
      // we have configured WB CNG
      if(_cng_WB_pt == payloadType) {
        // only set a marker bit when we change payload type to a non CNG
        return false;
      }
    }

    if(_cng_SWB_pt != -1) {
      // we have configured SWB CNG
      if(_cng_SWB_pt == payloadType) {
        // only set a marker bit when we change payload type to a non CNG
        return false;
      }
    }

    if(_cng_FB_pt != -1) {
      // we have configured SWB CNG
      if(_cng_FB_pt == payloadType) {
        // only set a marker bit when we change payload type to a non CNG
        return false;
      }
    }

    // payloadType differ
    if(_latest_pt == -1) {
      return true;
      //if(frameType != kAudioFrameCN) {
      //  // first packet and NOT CNG
      //  return true;

      //} else {
      //  // first packet and CNG
      //  _inband_VAD_active = true;
      //  return false;
      //}
    }

    // not first packet AND
    // not CNG AND
    // payloadType changed

    // set a marker bit when we change payload type
    markerBit = true;
  }

  // For G.723 G.729, AMR etc we can have inband VAD
  /*if(frameType == kAudioFrameCN) {
    _inband_VAD_active = true;

  } else if(_inband_VAD_active) {
    _inband_VAD_active = false;
    markerBit = true;
  }*/

  return markerBit;
}

WebRtc_Word16 AudioDataCombiner::BuildRTPheader(WebRtc_UWord8* dataBuffer,
    const WebRtc_Word8 payloadType, const bool markerBit,
    const WebRtc_UWord32 captureTimeStamp,
    const bool timeStampProvided /* = true */,
    const bool incSequenceNumber /* = true */) {
  assert(payloadType >= 0);

  dataBuffer[0] = static_cast<WebRtc_UWord8>(0x80);  // version 2
  dataBuffer[1] = static_cast<WebRtc_UWord8>(payloadType);

  if (markerBit) {
    dataBuffer[1] |= kRtpMarkerBitMask;  // MarkerBit is set
  }

  if (timeStampProvided) {
    _timeStamp = _startTimeStamp + captureTimeStamp;
  } else {
    // make a unique time stamp
    // we can't inc by the actual time, since then we increase the risk of back
    // timing.
    _timeStamp++;
  }

  ModuleRTPUtility::AssignUWord16ToBuffer(dataBuffer + 2, _sequenceNumber);
  ModuleRTPUtility::AssignUWord32ToBuffer(dataBuffer + 4, _timeStamp);
  ModuleRTPUtility::AssignUWord32ToBuffer(dataBuffer + 8, _ssrc);
  WebRtc_Word16 rtpHeaderLength = 12;

  if (incSequenceNumber) ++ _sequenceNumber;

  return rtpHeaderLength;
}

void AudioDataCombiner::SetStatus(bool enable) {
  _enable = enable;
}

bool AudioDataCombiner::SendPacket(uint8_t* data_buffer, uint8_t payload_type,
                                   uint32_t timestamp, uint16_t sequence_number, int payload_len) {
  if (_latest_pt && _latest_pt != payload_type) {
    SendPacketToTransport();
    //_latest_pt = payload_type;
  }

  if (_latest_seq && _latest_seq != sequence_number - 1) {
    SendPacketToTransport();
  }

  if (_wr_pos + payload_len > _max_data_size) {
    SendPacketToTransport();
  }

  PutData(data_buffer, (uint16_t)payload_len, timestamp, payload_type,
          sequence_number);

  if (_buffered_packet >= _max_buffered_packet) {
    _buffered_packet = 0;
    SendPacketToTransport();
  }

  

  return true;
}

int32_t AudioDataCombiner::SendPacketToTransport() {
  if (0 == _wr_pos) return _wr_pos;

  WebRtc_UWord8 dataBuffer[IP_PACKET_SIZE];
  bool markerBit = MarkerBit(_latest_pt);

  uint16_t rtpHeaderLength = 0;
  rtpHeaderLength += BuildRTPheader(dataBuffer, _latest_pt,
                     markerBit, _begin_timestamp);

  ModuleRTPUtility::AssignUWord16ToBuffer(dataBuffer + rtpHeaderLength,
                                          (uint16_t)_begin_seq);
  rtpHeaderLength += 2;

  if (rtpHeaderLength <= 2) {
    return -1;
  }

  memcpy(dataBuffer + rtpHeaderLength, _data_buffer, _wr_pos);

  _buffered_packet = 0;
  uint32_t ret = _data_callback->SendCombinedDataCallback(dataBuffer, _wr_pos + rtpHeaderLength);                
  _wr_pos = 0;
  _new_segment = true;
  return ret;
}

void AudioDataCombiner::PutData(const uint8_t *data_ptr, uint16_t data_len,
                                uint32_t timestamp, uint8_t payload_type, uint16_t sequence_number) {
  /*if (0 == _begin_timestamp) {
    _begin_timestamp = timestamp;
  }
  if (0 == _begin_seq) {
    _begin_seq = sequence_number;
  }*/

  if (_new_segment) {
    _begin_timestamp = timestamp;
    _begin_seq = sequence_number;    
    _new_segment = false;
    _latest_pt = payload_type;
  }

  // build segment header
  uint32_t ts_delta = timestamp - _begin_timestamp;

  uint8_t *buf = _data_buffer + _wr_pos;
  ModuleRTPUtility::AssignUWord16ToBuffer(buf, (uint16_t)ts_delta);
  ModuleRTPUtility::AssignUWord16ToBuffer(buf + 2, (uint16_t)data_len);

  uint16_t hdr_size = 4;
  memcpy(buf + hdr_size, data_ptr, data_len);
  _wr_pos += data_len + hdr_size;

  ++ _buffered_packet;  

  _latest_pt = payload_type;
  _latest_seq = sequence_number;
}

void AudioDataCombiner::SetSSRC(uint32_t ssrc) {
  _ssrc = ssrc;
}

void AudioDataCombiner::SetTimeStamp(uint32_t timeStamp) {
  _timeStamp = timeStamp;
}

void AudioDataCombiner::SetSequenceNumber(uint16_t sequenceNumber) {
  _sequenceNumber = sequenceNumber;
}

void AudioDataCombiner::IncreaseBufferPacket() {
  ++ _buffered_packet;
}

}

