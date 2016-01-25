#ifndef __AUDIO_DATA_SEPARATOR_H__
#define __AUDIO_DATA_SEPARATOR_H__

#include "modules/rtp_rtcp/interface/rtp_rtcp_defines.h"

namespace webrtc {
class RTPReceiverAudio;

class AudioDataSeparatorCallback {
public:
  virtual uint32_t SeparatedRtpCallback(const WebRtc_UWord8* payloadData,
    const WebRtc_UWord16 payloadSize) = 0;
};

class AudioDataSeparator {
public:
  AudioDataSeparator(AudioDataSeparatorCallback *callback);
  virtual ~AudioDataSeparator();

  void ParseSegmentPacket(const uint8_t* data, size_t data_len,
                          WebRtcRTPHeader* rtpHeader);


private:
  AudioDataSeparatorCallback *_rtp_receiver;
};


}



#endif // __AUDIO_DATA_SEPARATOR_H__
