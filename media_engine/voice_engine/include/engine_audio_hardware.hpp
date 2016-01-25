#ifndef __MEDIA_ENGINE_AUDIO_HARDWARE_H__
#define __MEDIA_ENGINE_AUDIO_HARDWARE_H__

#include "engine_base.hpp"
#include "engine_hardware.h"

class MediaEngineVoiceHardware {
public:
  // Factory for the VoEHardware sub-API. Increases an internal
  // reference counter if successful. Returns NULL if the API is not
  // supported or if construction fails.
  static MediaEngineVoiceHardware* GetInterface(MediaEngine* engine) {
    MediaEngineVoiceHardware *obj = 0;
    obj = new MediaEngineVoiceHardware();
    if (0 == obj) return 0;

    int ret = media_engine_get_hardware(engine->_eng_ptr, obj->_obj_ptr);
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
  static int Release(MediaEngineVoiceHardware *hdr) {
    assert(hdr);
    media_engine_release_hardware(hdr->_obj_ptr);
    delete hdr;
    return 0;
  }

public:
  // Gets the number of audio devices available for recording.
  int GetNumOfRecordingDevices(int& devices) {
    assert(_obj_ptr);
    return engine_hardware_get_num_record_dev(_obj_ptr, devices);
  }

  // Gets the number of audio devices available for playout.
  int GetNumOfPlayoutDevices(int& devices) {
    assert(_obj_ptr);
    return engine_hardware_get_num_play_dev(_obj_ptr, devices);
  }

  // Gets the name of a specific recording device given by an |index|.
  // On Windows Vista/7, it also retrieves an additional unique ID
  // (GUID) for the recording device.
  int GetRecordingDeviceName(int index, char strNameUTF8[128],
    char strGuidUTF8[128]) {
    assert(_obj_ptr);
    return engine_hardware_get_record_dev_name(_obj_ptr, index, strNameUTF8,
      strGuidUTF8);
  }

  // Gets the name of a specific playout device given by an |index|.
  // On Windows Vista/7, it also retrieves an additional unique ID
  // (GUID) for the playout device.
  int GetPlayoutDeviceName(int index, char strNameUTF8[128],
    char strGuidUTF8[128]) {
    assert(_obj_ptr);
    return engine_hardware_get_play_dev_name(_obj_ptr, index, strNameUTF8,
      strGuidUTF8);
  }

  // Sets the audio device used for recording.
  virtual int SetRecordingDevice(int index, StereoChannel recordingChannel = kStereoBoth) {
    assert(_obj_ptr);
    return engine_hardware_set_record_dev(_obj_ptr, index, recordingChannel);
  }

  // Sets the audio device used for playout.
  virtual int SetPlayoutDevice(int index) {
    assert(_obj_ptr);
    return engine_hardware_set_playout_dev(_obj_ptr, index);
  }

  virtual int InputDevice() {
    assert(_obj_ptr);
    int index = -3;
    if (engine_hardware_get_cur_record_dev(_obj_ptr, index))
      return -3;
    return index;
  }

  virtual int OutputDevice() {
    assert(_obj_ptr);
    int index = -3;
    if (engine_hardware_get_cur_playout_dev(_obj_ptr, index))
      return -3;
    return index;
  }
private:
  media_engine_hardware *_obj_ptr;
};


#endif // __MEDIA_ENGINE_AUDIO_HARDWARE_H__
