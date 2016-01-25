#ifndef __MEDIA_ENGINE_BASE_HPP__
#define __MEDIA_ENGINE_BASE_HPP__

#include "common_types.h"
extern "C" {
#include "engine_base.h"
#include "engine_hardware.h"
}

#include <assert.h>

#ifdef __cplusplus
using namespace webrtc;
#endif

class MediaEngineVoiceHardware;
class MediaEngineVoe;
class MediaEngineVoeCodec;
class MediaEngineNetwork;
class MediaEngineVoiceFile;
class MediaEngineAPM;
class EngineAudioVolume;
class MediaEngineDebug;

class MediaEngine {
public:
  friend class MediaEngineVoiceHardware;
  friend class MediaEngineVoe;
  friend class MediaEngineVoeCodec;
  friend class MediaEngineNetwork;
  friend class MediaEngineVoiceFile;
  friend class MediaEngineAPM;
  friend class EngineAudioVolume;
  friend class MediaEngineDebug;

  // Creates a VoiceEngine object, which can then be used to acquire
  // sub-APIs. Returns NULL on failure.
  static MediaEngine* Create() {
    MediaEngine * obj = 0;
    obj = new MediaEngine;    
    return obj;    
  }
  
  // Deletes a created VoiceEngine object and releases the utilized resources.
  // Note that if there are outstanding references held via other interfaces,
  // the voice engine instance will not actually be deleted until those
  // references have been released.
  static int Delete(MediaEngine*& voiceEngine) {
    int ret = media_engine_delete(voiceEngine->_eng_ptr);    
    delete voiceEngine;
    voiceEngine = 0;
    return ret;    
  }  

  // Specifies the amount and type of trace information which will be
  // created by the VoiceEngine.
  static int SetTraceFilter(const unsigned int filter, const unsigned int module) {
    return media_engine_set_trace_filter(filter, module);
  }

  // Sets the name of the trace file and enables non-encrypted trace messages.
  static int SetTraceFile(const char* fileNameUTF8, const bool addFileCounter = false) {
    return media_engine_set_trace_file(fileNameUTF8, addFileCounter);
  }
    
public:
  MediaEngine() : _eng_ptr(0) {
    int ret = media_engine_create(&_eng_ptr);
    if (ret) {
      assert(0);
    }
  }

  virtual ~MediaEngine() {
    assert(0 == _eng_ptr);
  }
  
  // Initiates all common parts of the MediaEngine; e.g. all
  // encoders/decoders, the sound card and core receiving components.  
  int Init() {
    assert(_eng_ptr);
    return media_engine_init(_eng_ptr);
  }

  // Terminates all VoiceEngine functions and releases allocated resources.
  virtual int Terminate() {
    assert(_eng_ptr);
    return media_engine_terminate(_eng_ptr);
  }  
 
private:
  media_engine *_eng_ptr;
};




#endif // __MEDIA_ENGINE_BASE_HPP__
