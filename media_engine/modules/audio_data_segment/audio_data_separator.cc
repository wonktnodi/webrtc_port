#include "audio_data_separator.h"
#include "modules/rtp_rtcp/source/rtp_receiver_audio.h"
#include "trace.h"

namespace webrtc {
static WebRtc_UWord16 BuildRTPheader(WebRtc_UWord8* dataBuffer,
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
  WebRtc_UWord16 rtpHeaderLength = 12;

  return rtpHeaderLength;
}

AudioDataSeparator::AudioDataSeparator(AudioDataSeparatorCallback *callback)
  : _rtp_receiver(callback) {
}

AudioDataSeparator::~AudioDataSeparator() {
}

void AudioDataSeparator::ParseSegmentPacket(const uint8_t* data, size_t data_len,
    WebRtcRTPHeader* rtpHeader) {
  uint16_t begin_seq = 0;
  uint16_t parsed_len = 0;
  const uint8_t *buf_ptr = data;

  begin_seq = (data[0] << 8) + data[1];
  buf_ptr += 2;
  parsed_len += 2;
  uint16_t seq_delta = 0;

  while(parsed_len < data_len) {
    WebRtcRTPHeader segment_rtp_hdr = *rtpHeader;
    uint16_t ts_delta = (buf_ptr[0] << 8) + buf_ptr[1];
    buf_ptr += 2;
    uint16_t packet_len = (buf_ptr[0] << 8) + buf_ptr[1];
    buf_ptr += 2;

    parsed_len += 4;

    segment_rtp_hdr.header.timestamp +=  ts_delta;
    segment_rtp_hdr.header.sequenceNumber = begin_seq + seq_delta;
    
    WebRtc_UWord8 buf[IP_PACKET_SIZE - 28];
    uint16_t max_packet_size = IP_PACKET_SIZE - 28;
    uint16_t hdr_len = BuildRTPheader(buf, segment_rtp_hdr.header.payloadType,
      segment_rtp_hdr.header.sequenceNumber, segment_rtp_hdr.header.markerBit,
      segment_rtp_hdr.header.timestamp, segment_rtp_hdr.header.ssrc);
    int rtp_data_len = max_packet_size - hdr_len;

    if (rtp_data_len < packet_len) {
      WEBRTC_TRACE(kTraceDebug, kTraceRtpRtcp, 0,
        "ReceivedRawPacket() invalid packet length");
      return;
    } else {
      memcpy(buf + hdr_len, buf_ptr, packet_len);
      buf_ptr += packet_len;
    }    

    if (_rtp_receiver)
      _rtp_receiver->SeparatedRtpCallback(buf, packet_len + hdr_len);
    parsed_len += packet_len;
    ++ seq_delta;
  }
}

}



