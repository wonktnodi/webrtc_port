#ifndef __AUDIO_DATA_COMBINER_H__
#define __AUDIO_DATA_COMBINER_H__

#include "media_engine/typedefs.h"
#include "modules/rtp_rtcp/interface/rtp_rtcp_defines.h"

namespace webrtc {

//class RTPSenderInterface;

class AudioDataCombinerCallback {
public:
  virtual uint32_t SendCombinedDataCallback(const WebRtc_UWord8* payloadData,
    const WebRtc_UWord16 payloadSize) = 0;
};

class AudioDataCombiner {
public:
  AudioDataCombiner(AudioDataCombinerCallback *data_callback);
  virtual ~AudioDataCombiner();

  // Enable/disable combining.
  void SetStatus(bool enable);

  // Returns true if we send the packet now, else it will add the packet
  // information to the queue and call TimeToSendPacket when it's time to send.
  bool SendPacket(uint8_t* data_buffer, uint8_t payload_type, uint32_t timestamp,
                  uint16_t sequence_number, int payload_len);

  void RegisterCNPaylodType(uint8_t nb_pt = -1, uint8_t wb_pt = -1,
                            uint8_t swb_pt = -1, uint8_t fb_pt = -1);
  
  void SetMaxBufferedPacket(uint16_t max_packet);

  int32_t SendPacketToTransport();

  void SetSSRC(uint32_t ssrc);

  void SetSequenceNumber(uint16_t sequenceNumber);

  void SetTimeStamp(uint32_t timeStamp);

  void IncreaseBufferPacket();

protected:  
  void PutData(const uint8_t *data_ptr, uint16_t data_len, uint32_t timestamp,
               uint8_t payload_type, uint16_t sequence_number);
  bool MarkerBit(const WebRtc_Word8 payloadType);

  WebRtc_Word16 BuildRTPheader(WebRtc_UWord8* dataBuffer,
    const WebRtc_Word8 payloadType,
    const bool markerBit,
    const WebRtc_UWord32 captureTimeStamp,
    const bool timeStampProvided = true,
    const bool incSequenceNumber = true);
private:
  struct SegmentHdr {
    SegmentHdr(uint16_t timestamp_offset, uint16_t length_in_bytes)
      : _timestamp_offset(timestamp_offset), _bytes(length_in_bytes) {
    }
    uint16_t _timestamp_offset;
    uint16_t _bytes;
  };

  bool _enable;
  //RTPSenderInterface * _rtp_sender;
  AudioDataCombinerCallback *_data_callback;
  bool _inband_VAD_active;
  uint8_t _cng_NB_pt;
  uint8_t _cng_WB_pt;
  uint8_t _cng_SWB_pt;
  uint8_t _cng_FB_pt;
  uint8_t  _latest_pt;

  uint16_t _begin_seq;
  uint16_t _latest_seq;
  uint32_t _begin_timestamp;

  uint8_t _data_buffer[IP_PACKET_SIZE - 28];  // // default is IP-v4/UDP
  uint16_t _wr_pos;
  //uint16_t _data_size;

  uint8_t _packet_combined;
  uint8_t _buffered_packet;

  uint16_t _max_data_size;
  uint16_t _max_buffered_packet;

  bool _new_segment;

  WebRtc_UWord32  _ssrc;
  WebRtc_UWord32  _timeStamp;
  WebRtc_UWord32  _startTimeStamp;
  WebRtc_UWord16  _sequenceNumber;
};

}


#endif // __AUDIO_DATA_COMBINER_H__
