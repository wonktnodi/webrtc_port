#ifndef __MEDIA_ENGINE_DEBUG_HPP__
#define __MEDIA_ENGINE_DEBUG_HPP__

#include "engine_debug.h"
#include "engine_base.hpp"

class MediaEngineDebug {
public:
  static MediaEngineDebug* GetInterface(MediaEngine *eng) {
    MediaEngineDebug *obj = 0;
    obj = new MediaEngineDebug();
    if (0 == obj) return 0;

    int ret = media_engine_get_debuger(eng->_eng_ptr, obj->_obj_ptr);
    if (ret) {
      delete obj;
      return 0;
    }

    return obj;
  }

  static int Release(MediaEngineDebug *hdr) {
    assert(hdr);
    media_engine_release_debuger(hdr->_obj_ptr);
    delete hdr;
    return 0;
  }

public:
  int StartRtpDump(int channel, const char* utf8_file_name, RTPDirections direction) {
    return media_engine_debug_strat_rtp_dump(_obj_ptr, channel, utf8_file_name, direction);
  }

  int StopRtpDump(int channel, RTPDirections direction) {
    return media_engine_debug_stop_rtp_dump(_obj_ptr, channel, direction);
  }

  int RTPDumpIsActive(int channel, RTPDirections direction) {
    return media_engine_debug_rtp_dump_active(_obj_ptr, channel, direction);
  }

  int WriteExternInfo(const char* utf8_file_name, const char *buf, int buf_len) {
    return media_engine_debug_write_rtp_dump_extern_info(
      _obj_ptr, utf8_file_name, buf, buf_len);
  }
private:
  media_engine_debuger *_obj_ptr;
};


#endif // __MEDIA_ENGINE_DEBUG_HPP__
