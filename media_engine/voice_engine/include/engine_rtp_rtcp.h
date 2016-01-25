#ifndef __MEDIA_ENGINE_RTP_RTCP_H__
#define __MEDIA_ENGINE_RTP_RTCP_H__

namespace webrtc {

// CallStatistics
struct CallStatistics {
  unsigned short fractionLost;
  unsigned int cumulativeLost;
  unsigned int extendedMax;
  unsigned int jitterSamples;
  int rttMs;
  int bytesSent;
  int packetsSent;
  int bytesReceived;
  int packetsReceived;
};

}  // namespace webrtc

#endif // __MEDIA_ENGINE_RTP_RTCP_H__
