#ifndef __MEDIA_ENGINE_NETWORK_IMPL_H__
#define __MEDIA_ENGINE_NETWORK_IMPL_H__

#include "common_types.h"

namespace webrtc {
class SharedData;
}

using namespace webrtc;

typedef struct engine_transport engine_transport;

class MediaEngineNetworkImpl {
public:
  virtual int ReceivedRTPPacket(int channel, const void* data, unsigned int length);
  virtual int ReceivedRawPacket(int channel, uint8_t bit_mask, uint8_t pt,
    uint16_t seq, uint32_t tm, uint32_t ssrc, const char * data, int len);
  virtual int ReceivedRTCPPacket(int channel, const void* data, unsigned int length);

  virtual int RegisterTransport(engine_transport *transport);

public:
  MediaEngineNetworkImpl(SharedData*);
  virtual ~MediaEngineNetworkImpl();

private:
  SharedData* _shared;  
};


#endif // __MEDIA_ENGINE_NETWORK_IMPL_H__
