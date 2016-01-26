#include "engine_base_impl.h"
#include "trace.h"
#include "critical_section_wrapper.h"
#include "signal_processing_library.h"
#include "modules/audio_device/audio_device_impl.h"
#include "transmit_mixer.h"
#include "output_mixer.h"

using namespace webrtc;

int MediaEngineImpl::AddRef() {
  return ++_ref_count;
}

// This implements the Release() method for all the inherited interfaces.
int MediaEngineImpl::Release() {
  int new_ref = --_ref_count;
  assert(new_ref >= 0);
  if (new_ref == 0) {
    WEBRTC_TRACE(kTraceApiCall, kTraceVoice, -1,
      "VoiceEngineImpl self deleting (voiceEngine=0x%p)",
      this);

    delete this;
  }

  return new_ref;
}

int MediaEngineImpl::Init() {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(instance_id(), -1),
    "Init");
  CriticalSectionScoped cs(crit_sec());

  WebRtcSpl_Init();

  if (statistics().Initialized()) {
    return 0;
  }

  // Create the internal ADM implementation.
  set_audio_device(AudioDeviceModuleImpl::Create(VoEId(instance_id(), -1),
    AudioDeviceModule::kWindowsWaveAudio));

  bool available(false);

  if (audio_device() == NULL) {
    SetLastError(VE_NO_MEMORY, kTraceCritical, "Init() failed to create the ADM");
    return -1;
  }

  // ADM initialization

  // Register the AudioObserver implementation
  if (audio_device()->RegisterEventObserver(this) != 0) {
    SetLastError(VE_AUDIO_DEVICE_MODULE_ERROR, kTraceWarning,
      "Init() failed to register event observer for the ADM");
  }

  // Register the AudioTransport implementation
  if (audio_device()->RegisterAudioCallback(this) != 0) {
    SetLastError(VE_AUDIO_DEVICE_MODULE_ERROR, kTraceWarning,
      "Init() failed to register audio callback for the ADM");
  }


  if (audio_device()->Init() != 0) {
    SetLastError(VE_AUDIO_DEVICE_MODULE_ERROR, kTraceError,
      "Init() failed to initialize the ADM");
    return -1;
  }

  // Initialize the default speaker
  if (audio_device()->SetPlayoutDevice(WEBRTC_VOICE_ENGINE_DEFAULT_DEVICE) != 0) {
    SetLastError(VE_AUDIO_DEVICE_MODULE_ERROR, kTraceInfo,
      "Init() failed to set the default output device");
  }
  if (audio_device()->SpeakerIsAvailable(&available) != 0) {
    SetLastError(VE_CANNOT_ACCESS_SPEAKER_VOL, kTraceInfo,
      "Init() failed to check speaker availability, trying to "
      "initialize speaker anyway");
  } else if (!available) {
    SetLastError(VE_CANNOT_ACCESS_SPEAKER_VOL, kTraceInfo,
      "Init() speaker not available, trying to initialize speaker "
      "anyway");
  }

  if (audio_device()->InitSpeaker() != 0) {
    SetLastError(VE_CANNOT_ACCESS_SPEAKER_VOL, kTraceInfo,
      "Init() failed to initialize the speaker");
  }

  // Initialize the default microphone
  if (audio_device()->SetRecordingDevice(WEBRTC_VOICE_ENGINE_DEFAULT_DEVICE) != 0) {
    SetLastError(VE_SOUNDCARD_ERROR, kTraceInfo,
      "Init() failed to set the default input device");
  }
  if (audio_device()->MicrophoneIsAvailable(&available) != 0) {
    SetLastError(VE_CANNOT_ACCESS_MIC_VOL, kTraceInfo,
      "Init() failed to check microphone availability, trying to "
      "initialize microphone anyway");
  } else if (!available) {
    SetLastError(VE_CANNOT_ACCESS_MIC_VOL, kTraceInfo,
      "Init() microphone not available, trying to initialize "
      "microphone anyway");
  }
  if (audio_device()->InitMicrophone() != 0) {
    SetLastError(VE_CANNOT_ACCESS_MIC_VOL, kTraceInfo,
      "Init() failed to initialize the microphone");
  }

  // Set number of channels
  if (audio_device()->StereoPlayoutIsAvailable(&available) != 0) {
    SetLastError(VE_SOUNDCARD_ERROR, kTraceWarning,
      "Init() failed to query stereo playout mode");
  }
  if (audio_device()->SetStereoPlayout(available) != 0) {
    SetLastError(VE_SOUNDCARD_ERROR, kTraceWarning,
      "Init() failed to set mono/stereo playout mode");
  }

  // TODO(andrew): These functions don't tell us whether stereo recording
  // is truly available. We simply set the AudioProcessing input to stereo
  // here, because we have to wait until receiving the first frame to
  // determine the actual number of channels anyway.
  //
  // These functions may be changed; tracked here:
  // http://code.google.com/p/webrtc/issues/detail?id=204
  audio_device()->StereoRecordingIsAvailable(&available);
  if (audio_device()->SetStereoRecording(available) != 0) {
    SetLastError(VE_SOUNDCARD_ERROR, kTraceWarning,
      "Init() failed to set mono/stereo recording mode");
  }

  // APM initialization done after sound card since we need
  // to know if we support stereo recording or not.

  // Create the AudioProcessing Module if it does not exist.

  if (audio_processing() == NULL) {
    set_audio_processing(AudioProcessing::Create(
      VoEId(instance_id(), -1)));
    if (audio_processing() == NULL) {
      SetLastError(VE_NO_MEMORY, kTraceCritical,
        "Init() failed to create the AP module");
      return -1;
    }
    // Ensure that mixers in both directions has access to the created APM
    transmit_mixer()->SetAudioProcessingModule(audio_processing());
    output_mixer()->SetAudioProcessingModule(audio_processing());

    if (audio_processing()->echo_cancellation()->set_device_sample_rate_hz(
      kVoiceEngineAudioProcessingDeviceSampleRateHz)) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set the device sample rate to 48K for AP "
        " module");
      return -1;
    }
    // Using 8 kHz as initial Fs. Might be changed already at first call.
    if (audio_processing()->set_sample_rate_hz(8000)) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set the sample rate to 8K for AP module");
      return -1;
    }

    // Assume mono until the audio frames are received from the capture
    // device, at which point this can be updated.
    if (audio_processing()->set_num_channels(1, 1) != 0) {
      SetLastError(VE_SOUNDCARD_ERROR, kTraceError,
        "Init() failed to set channels for the primary audio stream");
      return -1;
    }

    if (audio_processing()->set_num_reverse_channels(1) != 0) {
      SetLastError(VE_SOUNDCARD_ERROR, kTraceError,
        "Init() failed to set channels for the primary audio stream");
      return -1;
    }
    // high-pass filter
    if (audio_processing()->high_pass_filter()->Enable(
      WEBRTC_VOICE_ENGINE_HP_DEFAULT_STATE) != 0) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set the high-pass filter for AP module");
      return -1;
    }
    // Echo Cancellation
    if (audio_processing()->echo_cancellation()->
      enable_drift_compensation(false) != 0) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set drift compensation for AP module");
      return -1;
    }
    if (audio_processing()->echo_cancellation()->Enable(
      WEBRTC_VOICE_ENGINE_EC_DEFAULT_STATE)) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set echo cancellation state for AP module");
      return -1;
    }
    // Noise Reduction
    if (audio_processing()->noise_suppression()->set_level(
      (NoiseSuppression::Level) WEBRTC_VOICE_ENGINE_NS_DEFAULT_MODE) != 0) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set noise reduction level for AP module");
      return -1;
    }
    if (audio_processing()->noise_suppression()->Enable(
      WEBRTC_VOICE_ENGINE_NS_DEFAULT_STATE) != 0) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set noise reduction state for AP module");
      return -1;
    }
    // Automatic Gain control
    if (audio_processing()->gain_control()->
      set_analog_level_limits(kMinVolumeLevel,kMaxVolumeLevel) != 0) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set AGC analog level for AP module");
      return -1;
    }
    if (audio_processing()->gain_control()->set_mode(
      (GainControl::Mode) WEBRTC_VOICE_ENGINE_AGC_DEFAULT_MODE)
      != 0) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set AGC mode for AP module");
      return -1;
    }
    if (audio_processing()->gain_control()->Enable(
      WEBRTC_VOICE_ENGINE_AGC_DEFAULT_STATE) != 0) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set AGC state for AP module");
      return -1;
    }
    // VAD
    if (audio_processing()->voice_detection()->Enable(
      WEBRTC_VOICE_ENGINE_VAD_DEFAULT_STATE) != 0) {
      SetLastError(VE_APM_ERROR, kTraceError,
        "Init() failed to set VAD state for AP module");
      return -1;
    }
  }


  return statistics().SetInitialized();
}

int MediaEngineImpl::Terminate() {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(instance_id(), -1),
    "Terminate()");
  CriticalSectionScoped cs(crit_sec());
  return TerminateInternal();  
}

WebRtc_Word32 MediaEngineImpl::TerminateInternal() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(instance_id(), -1),
    "VoEBaseImpl::TerminateInternal()"); 

  if (audio_device() != NULL) {
    if (audio_device()->StopPlayout() != 0) {
      SetLastError(VE_SOUNDCARD_ERROR, kTraceWarning,
        "TerminateInternal() failed to stop playout");
    }
    if (audio_device()->StopRecording() != 0) {
      SetLastError(VE_SOUNDCARD_ERROR, kTraceWarning,
        "TerminateInternal() failed to stop recording");
    }
    if (audio_device()->RegisterEventObserver(NULL) != 0) {
      SetLastError(VE_AUDIO_DEVICE_MODULE_ERROR, kTraceWarning,
        "TerminateInternal() failed to de-register event observer "
        "for the ADM");
    }
    if (audio_device()->RegisterAudioCallback(NULL) != 0) {
      SetLastError(VE_AUDIO_DEVICE_MODULE_ERROR, kTraceWarning,
        "TerminateInternal() failed to de-register audio callback "
        "for the ADM");
    }
    if (audio_device()->Terminate() != 0) {
      SetLastError(VE_AUDIO_DEVICE_MODULE_ERROR, kTraceError,
        "TerminateInternal() failed to terminate the ADM");
    }

    set_audio_device(NULL);
  }

  // AP module

  if (audio_processing() != NULL) {
    transmit_mixer()->SetAudioProcessingModule(NULL);
    set_audio_processing(NULL);
  }
  return statistics().SetUnInitialized();
}

void MediaEngineImpl::OnErrorIsReported(const ErrorCode error) {
  //CriticalSectionScoped cs(&_callbackCritSect);
  //if (_voiceEngineObserver) {
  //  if (_voiceEngineObserverPtr) {
  //    int errCode(0);
  //    if (error == AudioDeviceObserver::kRecordingError) {
  //      errCode = VE_RUNTIME_REC_ERROR;
  //      WEBRTC_TRACE(kTraceInfo, kTraceVoice,
  //        VoEId(_shared->instance_id(), -1),
  //        "VoEBaseImpl::OnErrorIsReported() => VE_RUNTIME_REC_ERROR");
  //    } else if (error == AudioDeviceObserver::kPlayoutError) {
  //      errCode = VE_RUNTIME_PLAY_ERROR;
  //      WEBRTC_TRACE(kTraceInfo, kTraceVoice,
  //        VoEId(_shared->instance_id(), -1),
  //        "VoEBaseImpl::OnErrorIsReported() => "
  //        "VE_RUNTIME_PLAY_ERROR");
  //    }
  //    // Deliver callback (-1 <=> no channel dependency)
  //    _voiceEngineObserverPtr->CallbackOnError(-1, errCode);
  //  }
  //}
}

void MediaEngineImpl::OnWarningIsReported(const WarningCode warning) {
  //CriticalSectionScoped cs(&_callbackCritSect);
  //if (_voiceEngineObserver) {
  //  if (_voiceEngineObserverPtr) {
  //    int warningCode(0);
  //    if (warning == AudioDeviceObserver::kRecordingWarning) {
  //      warningCode = VE_RUNTIME_REC_WARNING;
  //      WEBRTC_TRACE(kTraceInfo, kTraceVoice,
  //        VoEId(_shared->instance_id(), -1),
  //        "VoEBaseImpl::OnErrorIsReported() => "
  //        "VE_RUNTIME_REC_WARNING");
  //    } else if (warning == AudioDeviceObserver::kPlayoutWarning) {
  //      warningCode = VE_RUNTIME_PLAY_WARNING;
  //      WEBRTC_TRACE(kTraceInfo, kTraceVoice,
  //        VoEId(_shared->instance_id(), -1),
  //        "VoEBaseImpl::OnErrorIsReported() => "
  //        "VE_RUNTIME_PLAY_WARNING");
  //    }
  //    // Deliver callback (-1 <=> no channel dependency)
  //    _voiceEngineObserverPtr->CallbackOnError(-1, warningCode);
  //  }
  //}
}

WebRtc_Word32 MediaEngineImpl::RecordedDataIsAvailable(
  const void* audioSamples,
  const WebRtc_UWord32 nSamples,
  const WebRtc_UWord8 nBytesPerSample,
  const WebRtc_UWord8 nChannels,
  const WebRtc_UWord32 samplesPerSec,
  const WebRtc_UWord32 totalDelayMS,
  const WebRtc_Word32 clockDrift,
  const WebRtc_UWord32 currentMicLevel,
  WebRtc_UWord32& newMicLevel) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(instance_id(), -1),
    "VoEBaseImpl::RecordedDataIsAvailable(nSamples=%u, "
    "nBytesPerSample=%u, nChannels=%u, samplesPerSec=%u, "
    "totalDelayMS=%u, clockDrift=%d, currentMicLevel=%u)",
    nSamples, nBytesPerSample, nChannels, samplesPerSec,
    totalDelayMS, clockDrift, currentMicLevel);

  assert(transmit_mixer() != NULL);
  assert(audio_device() != NULL);
   

  bool isAnalogAGC(false);
  WebRtc_UWord32 maxVolume(0);
  WebRtc_UWord16 currentVoEMicLevel(0);  
  WebRtc_UWord32 newVoEMicLevel(0);

  if (audio_processing() &&
    (audio_processing()->gain_control()->mode() == GainControl::kAdaptiveAnalog)) {
      isAnalogAGC = true;
  }

  // Will only deal with the volume in adaptive analog mode
  if (isAnalogAGC) {
    // Scale from ADM to VoE level range
    if (audio_device()->MaxMicrophoneVolume(&maxVolume) == 0) {
      if (0 != maxVolume) {
        currentVoEMicLevel = (WebRtc_UWord16) ((currentMicLevel
          * kMaxVolumeLevel + (int) (maxVolume / 2))
          / (maxVolume));
      }
    }
    // We learned that on certain systems (e.g Linux) the currentVoEMicLevel
    // can be greater than the maxVolumeLevel therefore
    // we are going to cap the currentVoEMicLevel to the maxVolumeLevel
    // and change the maxVolume to currentMicLevel if it turns out that
    // the currentVoEMicLevel is indeed greater than the maxVolumeLevel.
    if (currentVoEMicLevel > kMaxVolumeLevel) {
      currentVoEMicLevel = kMaxVolumeLevel;
      maxVolume = currentMicLevel;
    }
  }

  // Keep track if the MicLevel has been changed by the AGC, if not,
  // use the old value AGC returns to let AGC continue its trend,
  // so eventually the AGC is able to change the mic level. This handles
  // issues with truncation introduced by the scaling.
  if (_oldMicLevel == currentMicLevel) {
    currentVoEMicLevel = (WebRtc_UWord16) _oldVoEMicLevel;
  }

  // Perform channel-independent operations
  // (APM, mix with file, record to file, mute, etc.)
  transmit_mixer()->PrepareDemux(audioSamples, nSamples, nChannels,
    samplesPerSec, static_cast<WebRtc_UWord16>(totalDelayMS), clockDrift,
    currentVoEMicLevel);

  // Copy the audio frame to each sending channel and perform
  // channel-dependent operations (file mixing, mute, etc.) to prepare
  // for encoding.
  transmit_mixer()->DemuxAndMix();
  // Do the encoding and packetize+transmit the RTP packet when encoding
  // is done.
  transmit_mixer()->EncodeAndSend();

  // Will only deal with the volume in adaptive analog mode
  if (isAnalogAGC) {
    // Scale from VoE to ADM level range
    newVoEMicLevel = transmit_mixer()->CaptureLevel();
    if (newVoEMicLevel != currentVoEMicLevel) {
      // Add (kMaxVolumeLevel/2) to round the value
      newMicLevel = (WebRtc_UWord32) ((newVoEMicLevel * maxVolume
        + (int) (kMaxVolumeLevel / 2)) / (kMaxVolumeLevel));
    } else {
      // Pass zero if the level is unchanged
      newMicLevel = 0;
    }

    // Keep track of the value AGC returns
    _oldVoEMicLevel = newVoEMicLevel;
    _oldMicLevel = currentMicLevel;
  }

  return 0;
}

WebRtc_Word32 MediaEngineImpl::NeedMorePlayData(
  const WebRtc_UWord32 nSamples,
  const WebRtc_UWord8 nBytesPerSample,
  const WebRtc_UWord8 nChannels,
  const WebRtc_UWord32 samplesPerSec,
  void* audioSamples,
  WebRtc_UWord32& nSamplesOut) {
  WEBRTC_TRACE(kTraceStream, kTraceVoice, VoEId(instance_id(), -1),
    "VoEBaseImpl::NeedMorePlayData(nSamples=%u, "
    "nBytesPerSample=%d, nChannels=%d, samplesPerSec=%u)",
    nSamples, nBytesPerSample, nChannels, samplesPerSec);

  assert(output_mixer() != NULL);

  // TODO(andrew): if the device is running in mono, we should tell the mixer
  // here so that it will only request mono from AudioCodingModule.
  // Perform mixing of all active participants (channel-based mixing)
  output_mixer()->MixActiveChannels();

  // Additional operations on the combined signal
  output_mixer()->DoOperationsOnCombinedSignal();

  // Retrieve the final output mix (resampled to match the ADM)
  output_mixer()->GetMixedAudio(samplesPerSec, nChannels,
    &_audioFrame);

  assert(static_cast<int>(nSamples) == _audioFrame.samples_per_channel_);
  assert(samplesPerSec ==
    static_cast<WebRtc_UWord32>(_audioFrame.sample_rate_hz_));

  // Deliver audio (PCM) samples to the ADM
  memcpy(
    (WebRtc_Word16*) audioSamples,
    (const WebRtc_Word16*) _audioFrame.data_,
    sizeof(WebRtc_Word16) * (_audioFrame.samples_per_channel_
    * _audioFrame.num_channels_));

  nSamplesOut = _audioFrame.samples_per_channel_;

  return 0;
}
