#ifndef __MEDIA_ENGINE_VOE_APM_H__
#define __MEDIA_ENGINE_VOE_APM_H__

#include "common_types.h"

namespace webrtc {
  class SharedData;
}

using namespace webrtc;

class VoEAudioProcessingImpl {
public:
  VoEAudioProcessingImpl(SharedData* shared);
  virtual ~VoEAudioProcessingImpl();

  // Sets Noise Suppression (NS) status and mode.
  // The NS reduces noise in the microphone signal.
  virtual int SetNsStatus(bool enable, NsModes mode = kNsUnchanged);

  // Gets the NS status and mode.
  virtual int GetNsStatus(bool& enabled, NsModes& mode);

  // Sets the Automatic Gain Control (AGC) status and mode.
  // The AGC adjusts the microphone signal to an appropriate level.
  virtual int SetAgcStatus(bool enable, AgcModes mode = kAgcUnchanged);

  // Gets the AGC status and mode.
  virtual int GetAgcStatus(bool& enabled, AgcModes& mode);

  // Sets the AGC configuration.
  // Should only be used in situations where the working environment
  // is well known.
  virtual int SetAgcConfig(const AgcConfig config);

  // Gets the AGC configuration.
  virtual int GetAgcConfig(AgcConfig& config);

  // Sets the Echo Control (EC) status and mode.
  // The EC mitigates acoustic echo where a user can hear their own
  // speech repeated back due to an acoustic coupling between the
  // speaker and the microphone at the remote end.
  virtual int SetEcStatus(bool enable, EcModes mode = kEcUnchanged);

  // Gets the EC status and mode.
  virtual int GetEcStatus(bool& enabled, EcModes& mode);

  // Modifies settings for the AEC designed for mobile devices (AECM).
  virtual int SetAecmMode(AecmModes mode = kAecmSpeakerphone,
    bool enableCNG = true);

  // Gets settings for the AECM.
  virtual int GetAecmMode(AecmModes& mode, bool& enabledCNG);

  // Sets status and mode of the receiving-side (Rx) NS.
  // The Rx NS reduces noise in the received signal for the specified
  // |channel|. Intended for advanced usage only.
  virtual int SetRxNsStatus(int channel, bool enable, NsModes mode = kNsUnchanged);

  // Gets status and mode of the receiving-side NS.
  virtual int GetRxNsStatus(int channel, bool& enabled, NsModes& mode);

  // Sets status and mode of the receiving-side (Rx) AGC.
  // The Rx AGC adjusts the received signal to an appropriate level
  // for the specified |channel|. Intended for advanced usage only.
  virtual int SetRxAgcStatus(int channel, bool enable, AgcModes mode = kAgcUnchanged);

  // Gets status and mode of the receiving-side AGC.
  virtual int GetRxAgcStatus(int channel, bool& enabled, AgcModes& mode);

  // Modifies the AGC configuration on the receiving side for the
  // specified |channel|.
  virtual int SetRxAgcConfig(int channel, const AgcConfig config);

  // Gets the AGC configuration on the receiving side.
  virtual int GetRxAgcConfig(int channel, AgcConfig& config);

  virtual int EnableHighPassFilter(bool enable);
  virtual bool IsHighPassFilterEnabled();

  virtual int SetVADStatus(int channel, bool enable, VadModes mode = kVadConventional,
    bool disableDTX = false);

  virtual int GetVADStatus(int channel, bool& enabled, VadModes& mode,
    bool& disabledDTX);

private:
  SharedData* _shared;
  bool _isAecMode;
};

#endif // __MEDIA_ENGINE_VOE_APM_H__
