#ifndef __MEDIA_ENGINE_DEBUG_IMPL_H__
#define __MEDIA_ENGINE_DEBUG_IMPL_H__

#include "common_types.h"

namespace webrtc {

class SharedData;

}

using namespace webrtc;

class MediaEngineDebugImpl {
public:
  MediaEngineDebugImpl(SharedData* shared);
  virtual ~MediaEngineDebugImpl();

  int StartRTPDump(int channel, const char fileNameUTF8[1024],
    RTPDirections direction);
  int StopRTPDump(int channel, RTPDirections direction);
  int RTPDumpIsActive(int channel, RTPDirections direction);

private:
  SharedData* _shared;
};


#endif // __MEDIA_ENGINE_DEBUG_IMPL_H__
