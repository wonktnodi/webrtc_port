#ifndef __MEDIA_ENGINE_AUDIO_PROCESSING_HPP__
#define __MEDIA_ENGINE_AUDIO_PROCESSING_HPP__

#include "engine_base.hpp"
#include "engine_audio_processing.h"

class MediaEngineAPM {
public:
  static MediaEngineAPM* GetInterface(MediaEngine* engine) {
    MediaEngineAPM *obj = 0;
    obj = new MediaEngineAPM();
    if (0 == obj) return 0;

    int ret = media_engine_get_apm(engine->_eng_ptr, obj->_obj_ptr);
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
  static int Release(MediaEngineAPM *hdr) {
    assert(hdr);
    media_engine_release_apm(hdr->_obj_ptr);
    delete hdr;
    return 0;
  }

public:
  // Sets Noise Suppression (NS) status and mode.
  // The NS reduces noise in the microphone signal.
  virtual int SetNsStatus(bool enable, NsModes mode = kNsUnchanged) {
    return media_engine_apm_set_ns_status(_obj_ptr, enable, mode);
  }

  // Gets the NS status and mode.
  virtual int GetNsStatus(bool& enabled, NsModes& mode) {
    int ret_mode, ret;
    ret = media_engine_apm_get_ns_status(_obj_ptr, enabled, ret_mode);
    mode = (NsModes) ret_mode;
    return ret;
  }

  // Sets the Automatic Gain Control (AGC) status and mode.
  // The AGC adjusts the microphone signal to an appropriate level.
  virtual int SetAgcStatus(bool enable, AgcModes mode = kAgcUnchanged) {
    return media_engine_apm_set_agc_status(_obj_ptr, enable, mode);
  }

  // Gets the AGC status and mode.
  virtual int GetAgcStatus(bool& enabled, AgcModes& mode) {
    int ret_mode, ret;
    ret = media_engine_apm_get_agc_status(_obj_ptr, enabled, ret_mode);
    mode = (AgcModes) ret_mode;
    return ret;
  }
  
  // Sets the Echo Control (EC) status and mode.
  // The EC mitigates acoustic echo where a user can hear their own
  // speech repeated back due to an acoustic coupling between the
  // speaker and the microphone at the remote end.
  virtual int SetEcStatus(bool enable, EcModes mode = kEcUnchanged) {
    return media_engine_apm_set_ec_status(_obj_ptr, enable, mode);
  }

  // Gets the EC status and mode.
  virtual int GetEcStatus(bool& enabled, EcModes& mode) {
    int ret_mode, ret;
    ret = media_engine_apm_get_ec_status(_obj_ptr, enabled, ret_mode);
    mode = (EcModes) ret_mode;
    return ret;
  }

  // Modifies settings for the AEC designed for mobile devices (AECM).
  virtual int SetAecmMode(AecmModes mode = kAecmSpeakerphone,
    bool enableCNG = true) {
    return media_engine_apm_set_aecm_mode(_obj_ptr, mode, enableCNG);
  }

  // Gets settings for the AECM.
  virtual int GetAecmMode(AecmModes& mode, bool& enabledCNG) {
    int ret_mode, ret;
    ret = media_engine_apm_get_aecm_mode(_obj_ptr, ret_mode, enabledCNG);
    mode = (AecmModes) ret_mode;
    return ret;
  }

  // Sets status and mode of the receiving-side (Rx) NS.
  // The Rx NS reduces noise in the received signal for the specified
  // |channel|. Intended for advanced usage only.
  virtual int SetRxNsStatus(int channel, bool enable, NsModes mode = kNsUnchanged) {
    return media_engine_apm_set_rx_ns_status(_obj_ptr, channel, enable, mode);
  }

  // Gets status and mode of the receiving-side NS.
  virtual int GetRxNsStatus(int channel, bool& enabled, NsModes& mode) {
    int ret_mode, ret;
    ret = media_engine_apm_get_rx_ns_status(_obj_ptr, channel, enabled, ret_mode);
    mode = (NsModes) ret_mode;
    return ret;
  }

  // Sets status and mode of the receiving-side (Rx) AGC.
  // The Rx AGC adjusts the received signal to an appropriate level
  // for the specified |channel|. Intended for advanced usage only.
  virtual int SetRxAgcStatus(int channel, bool enable, AgcModes mode = kAgcUnchanged) {
    return media_engine_apm_set_rx_agc_status(_obj_ptr, channel, enable, mode);
  }

  // Gets status and mode of the receiving-side AGC.
  virtual int GetRxAgcStatus(int channel, bool& enabled, AgcModes& mode) {
    int ret_mode, ret;
    ret = media_engine_apm_get_rx_agc_status(_obj_ptr, channel, enabled, ret_mode);
    mode = (AgcModes) ret_mode;
    return ret;
  }

  // Enables a high pass filter on the capture signal. This removes DC bias
  // and low-frequency noise. Recommended to be enabled.
  virtual int EnableHighPassFilter(bool enable) {
    return media_engine_apm_enable_hpf(_obj_ptr, enable);
  }
  virtual bool IsHighPassFilterEnabled() {
    bool ret;
    media_engine_apm_is_hpf_enabled(_obj_ptr, ret);
    return ret;
  }

  // Sets the VAD/DTX (silence suppression) status and |mode| for a
  // specified |channel|. Disabling VAD (through |enable|) will also disable
  // DTX; it is not necessary to explictly set |disableDTX| in this case.
  virtual int SetVADStatus(int channel, bool enable,
    VadModes mode = kVadConventional, bool disableDTX = false) {
    return media_engine_apm_set_vad_status(_obj_ptr, channel,
      enable, mode, disableDTX);
  }

  // Gets the VAD/DTX status and |mode| for a specified |channel|.
  virtual int GetVADStatus(int channel, bool& enabled, VadModes& mode,
    bool& disabledDTX) {
    int tmp_mode = 0, ret = 0;
    ret = media_engine_apm_get_vad_status(_obj_ptr, channel, enabled,
      tmp_mode, disabledDTX);
    mode = (VadModes)tmp_mode;
    return ret;
  }

private:
  media_engine_apm *_obj_ptr;
};


#endif // __MEDIA_ENGINE_AUDIO_PROCESSING_HPP__
