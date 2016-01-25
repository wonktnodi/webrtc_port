#ifndef __MEDIA_ENGINE_VOE_FILE_HPP__
#define __MEDIA_ENGINE_VOE_FILE_HPP__

#include "engine_base.hpp"
#include "engine_voe_file.h"

class MediaEngineVoiceFile {
public:
  static MediaEngineVoiceFile* GetInterface(MediaEngine *eng) {
    MediaEngineVoiceFile *obj = 0;
    obj = new MediaEngineVoiceFile();
    if (0 == obj) return 0;

    int ret = media_engine_get_voe_file(eng->_eng_ptr, obj->_obj_ptr);
    if (ret) {
      delete obj;
      return 0;
    }

    return obj;
  }

  static int Release(MediaEngineVoiceFile *file) {
    assert(file);
    media_engine_release_voe_file(file->_obj_ptr);
    delete file;
    return 0;
  }

public:
  int StartPlayAsMic(int channel, const char* mic_file_utf8, bool loop,
    bool mix_with_mic, int format, float volume_scale  = 1.0f) {
    return media_engine_voice_start_play_file_as_mic(_obj_ptr, channel,
      mic_file_utf8, loop, mix_with_mic, format, volume_scale);
  }

  int StopPlayAsMic(int channel) {
    return media_engine_voice_stop_play_file_as_mic(_obj_ptr, channel);
  }

  // Returns whether the |channel| is currently playing a file as microphone.
  int IsPlayingFileAsMicrophone(int channel) {
    return media_engine_voice_is_play_file_as_mic(_obj_ptr, channel);
  }

  int StartPlayLocally(int channel,
    const char *file_name_utf8, bool loop, int format, float volume_scale = 1.0f) {
    return media_engine_voice_start_play_file_locally(_obj_ptr, channel,
      file_name_utf8, loop, format, volume_scale);
  }

  int StopPlayLocally(int channel) {
    return media_engine_voice_stop_play_file_locally(_obj_ptr, channel);
  }

  // Returns the current file playing state for a specific |channel|.
  int IsPlayingFileLocally(int channel) {
    return media_engine_voice_is_play_file_locally(_obj_ptr, channel);
  }

  int StartRecordPlayout(int channel, const char* file_name_utf8) {
    return media_engine_voice_start_record_playout(
      _obj_ptr, channel, file_name_utf8);
  }

  int StopRecordPlayout(int channel) {
    return media_engine_voice_stop_record_playout(_obj_ptr, channel);
  }

  // Record microphone signal to file

  int StartRecordingMicrophone(const char* fileNameUTF8, int maxSizeBytes = -1) {
    return media_engine_voice_start_record_mircophone(_obj_ptr, fileNameUTF8, 0, maxSizeBytes);    
  }

  int StopRecordingMicrophone() {
    return media_engine_voice_stop_record_mircophone(_obj_ptr);
  }

private:
  media_engine_voice_file *_obj_ptr;
};

#endif // __MEDIA_ENGINE_VOE_FILE_HPP__
