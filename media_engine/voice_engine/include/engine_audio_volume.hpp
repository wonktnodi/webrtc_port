#ifndef __MEDIA_ENGINE_AUDIO_VOLUME_HPP__
#define __MEDIA_ENGINE_AUDIO_VOLUME_HPP__

#include "engine_base.hpp"
#include "engine_audio_volume.h" 

class EngineAudioVolume {
public:
  // Factory for the VoEVolumeControl sub-API. Increases an internal
  // reference counter if successful. Returns NULL if the API is not
  // supported or if construction fails.
  static EngineAudioVolume* GetInterface(MediaEngine* engine) {
    EngineAudioVolume *obj = 0;
    obj = new EngineAudioVolume();
    if (0 == obj) return 0;

    int ret = media_engine_get_voice_volume(engine->_eng_ptr, obj->_obj_ptr);
    if (ret) {
      delete obj;
      return 0;
    }

    return obj;
  }

  // Releases the VoEVolumeControl sub-API and decreases an internal
  // reference counter. Returns the new reference count. This value should
  // be zero for all sub-API:s before the VoiceEngine object can be safely
  // deleted.
  static int Release(EngineAudioVolume *vol) {
    assert(vol);
    media_engine_release_voice_volume(vol->_obj_ptr);
    delete vol;
    return 0;
  }

  // Sets the speaker |volume| level. Valid range is [0,255].
  virtual int SetSpeakerVolume(unsigned int volume) {
    return media_engine_set_speaker_volume(_obj_ptr, volume);
  }

  // Gets the speaker |volume| level.
  virtual int GetSpeakerVolume(unsigned int& volume) {
    return media_engine_get_speaker_volume(_obj_ptr, volume);
  }

  // Mutes the speaker device completely in the operating system.
  virtual int SetSystemOutputMute(bool enable) {
    return media_engine_set_system_output_mute(_obj_ptr, enable);
  }

  // Gets the output device mute state in the operating system.
  virtual int GetSystemOutputMute(bool &enabled) {
    return media_engine_get_system_output_mute(_obj_ptr, enabled);
  }

  // Sets the microphone volume level. Valid range is [0,255].
  virtual int SetMicVolume(unsigned int volume) {
    return media_engine_set_mic_volume(_obj_ptr, volume);
  }

  // Gets the microphone volume level.
  virtual int GetMicVolume(unsigned int& volume) {
    return media_engine_get_mic_volume(_obj_ptr, volume);
  }

  // Mutes the microphone input signal completely without affecting
  // the audio device volume.
  virtual int SetInputMute(int channel, bool enable) {
    return media_engine_set_input_mute(_obj_ptr, channel, enable);
  }

  // Gets the current microphone input mute state.
  virtual int GetInputMute(int channel, bool& enabled) {
    return media_engine_get_input_mute(_obj_ptr, channel, enabled);
  }

  // Mutes the microphone device completely in the operating system.
  virtual int SetSystemInputMute(bool enable) {
    return media_engine_set_system_input_mute(_obj_ptr, enable);
  }

  // Gets the mute state of the input device in the operating system.
  virtual int GetSystemInputMute(bool& enabled) {
    return media_engine_get_system_input_mute(_obj_ptr, enabled);
  }

  // Gets the microphone speech |level|, mapped non-linearly to the range
  // [0,9].
  virtual int GetSpeechInputLevel(unsigned int& level) {
    return media_engine_get_speech_input_level(_obj_ptr, level);
  }

  // Gets the speaker speech |level|, mapped non-linearly to the range
  // [0,9].
  virtual int GetSpeechOutputLevel(int channel, unsigned int& level) {
    return media_engine_get_speech_output_level(_obj_ptr, channel, level);
  }

  // Gets the microphone speech |level|, mapped linearly to the range
  // [0,32768].
  virtual int GetSpeechInputLevelFullRange(unsigned int& level) {
    return media_engine_get_speech_input_level_range(_obj_ptr, level);
  }

  // Gets the speaker speech |level|, mapped linearly to the range [0,32768].
  virtual int GetSpeechOutputLevelFullRange(
    int channel, unsigned int& level) {
    return media_engine_get_speech_output_level_range(_obj_ptr, channel, level);
  }

  // Sets a volume |scaling| applied to the outgoing signal of a specific
  // channel. Valid scale range is [0.0, 10.0].
  virtual int SetChannelOutputVolumeScaling(int channel, float scaling) {
    return media_engine_set_channel_output_volume_scaling(_obj_ptr, channel, scaling);
  }

  // Gets the current volume scaling for a specified |channel|.
  virtual int GetChannelOutputVolumeScaling(int channel, float& scaling) {
    return media_engine_get_channel_output_volume_scaling(_obj_ptr, channel, scaling);
  }

  // Scales volume of the |left| and |right| channels independently.
  // Valid scale range is [0.0, 1.0].
  virtual int SetOutputVolumePan(int channel, float left, float right) {
    return meida_engine_set_output_volume_pan(_obj_ptr, channel, left, right);
  }

  // Gets the current left and right scaling factors.
  virtual int GetOutputVolumePan(int channel, float& left, float& right) {
    return meida_engine_get_output_volume_pan(_obj_ptr, channel, left, right);
  }

private:
  media_engine_voice_volume *_obj_ptr;
};



#endif // __MEDIA_ENGINE_AUDIO_VOLUME_HPP__
