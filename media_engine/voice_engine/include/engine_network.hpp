#ifndef __MEDIA_ENGINE_NETWORK_HPP__
#define __MEDIA_ENGINE_NETWORK_HPP__

#include "engine_base.hpp"
#include "engine_network.h"

class MediaEngineNetwork {
public:
  // Factory for the VoEHardware sub-API. Increases an internal
  // reference counter if successful. Returns NULL if the API is not
  // supported or if construction fails.
  static MediaEngineNetwork* GetInterface(MediaEngine* engine) {
    MediaEngineNetwork *obj = 0;
    obj = new MediaEngineNetwork();
    if (0 == obj) return 0;

    int ret = media_engine_get_network(engine->_eng_ptr, obj->_obj_ptr);
    if (ret) {
      delete obj;
      return 0;
    }

    return obj;
  }

  // Releases the VoEHardware sub-API and decreases an internal
  // reference counter. Returns the new reference count. This value should
  // be zero for all sub-API:s before the VoiceEngine object can be safely
  // deleted.
  static int Release(MediaEngineNetwork *net) {
    assert(net);
    media_engine_release_network(net->_obj_ptr);
    delete net;
    return 0;
  }

public:
  int RegisterTransport(engine_transport *transport) {
    return media_engine_network_register_transport(_obj_ptr, transport);
  }

  int InComintRawRtpData(int channel, uint8_t bit_mask, uint8_t pt, uint16_t seq,
    uint32_t tm, uint32_t ssrc, const char * data, int len) {
    return media_engine_network_income_rtp_raw_data(_obj_ptr, channel,
      bit_mask, pt, seq, tm, ssrc, data, len);
  }

  int InComingRtpData(int channel, const char* data, int len) {
    return media_engine_network_income_rtp_data(_obj_ptr, channel, data, len);
  }

  int InComingRtcpData(int channel, const char* data, int len) {
    return media_engine_network_income_rtcp_data(_obj_ptr, channel, data, len);
  }
private:
  media_engine_network *_obj_ptr;
};


#endif // __MEDIA_ENGINE_NETWORK_HPP__
