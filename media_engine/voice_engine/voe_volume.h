#ifndef __MEDIA_ENGINE_VOE_VOLUME_H__
#define __MEDIA_ENGINE_VOE_VOLUME_H__

#include "common_types.h"

namespace webrtc {
  class SharedData;
}

using namespace webrtc;

class VoEAudioVolumeImpl {
public:
  VoEAudioVolumeImpl(SharedData* shared);
  virtual ~VoEAudioVolumeImpl();

  virtual int SetSpeakerVolume(unsigned int volume);

  virtual int GetSpeakerVolume(unsigned int& volume);

  virtual int SetSystemOutputMute(bool enable);

  virtual int GetSystemOutputMute(bool& enabled);

  virtual int SetMicVolume(unsigned int volume);

  virtual int GetMicVolume(unsigned int& volume);

  virtual int SetInputMute(int channel, bool enable);

  virtual int GetInputMute(int channel, bool& enabled);

  virtual int SetSystemInputMute(bool enable);

  virtual int GetSystemInputMute(bool& enabled);

  virtual int GetSpeechInputLevel(unsigned int& level);

  virtual int GetSpeechOutputLevel(int channel, unsigned int& level);

  virtual int GetSpeechInputLevelFullRange(unsigned int& level);

  virtual int GetSpeechOutputLevelFullRange(int channel,
    unsigned int& level);

  virtual int SetChannelOutputVolumeScaling(int channel, float scaling);

  virtual int GetChannelOutputVolumeScaling(int channel, float& scaling);

  virtual int SetOutputVolumePan(int channel, float left, float right);

  virtual int GetOutputVolumePan(int channel, float& left, float& right);

private:
  SharedData* _shared;
};


#endif // __MEDIA_ENGINE_VOE_VOLUME_H__
