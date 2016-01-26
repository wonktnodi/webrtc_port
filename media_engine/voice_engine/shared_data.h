/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef __MEDIA_ENGINE_SHARED_DATA_H__
#define __MEDIA_ENGINE_SHARED_DATA_H__

#include <stdint.h>
#include "voice_engine_defines.h"

#include "channel_manager.h"
#include "statistics.h"
#include "process_thread.h"

#include "audio_device.h"
#include "audio_processing.h"

class ProcessThread;

typedef struct engine_transport engine_transport;

namespace webrtc {
class CriticalSectionWrapper;

namespace voe {
class TransmitMixer;
class OutputMixer;
}
using namespace voe;

class SharedData {
public:
    // Public accessors.
    WebRtc_UWord32 instance_id() const { return _instanceId; }
    CriticalSectionWrapper* crit_sec() { return _apiCritPtr; }

    Statistics& statistics() { return _engineStatistics; }
    ProcessThread* process_thread() { return _moduleProcessThreadPtr; }

    void set_audio_device(AudioDeviceModule* audio_device);
    AudioDeviceModule* audio_device() { return _audioDevicePtr; }
    bool ext_recording() const { return _externalRecording; }
    void set_ext_recording(bool value) { _externalRecording = value; }
    bool ext_playout() const { return _externalPlayout; }
    void set_ext_playout(bool value) { _externalPlayout = value; }    
    
    ChannelManager& channel_manager() { return _channelManager; }
    
    TransmitMixer* transmit_mixer() { return _transmitMixerPtr; }
 
    AudioProcessing* audio_processing() { return _audioProcessingModulePtr; }
    void set_audio_processing(AudioProcessing* audio_processing);
    
    OutputMixer* output_mixer() { return _outputMixerPtr; }    
    
    Transport *external_transport() { return &_externalTransportPtr; }
    void set_external_transport(engine_transport *transport);

    /*AudioDeviceModule::AudioLayer audio_device_layer() const {
      return _audioDeviceLayer;
    }
    void set_audio_device_layer(AudioDeviceModule::AudioLayer layer) {
      _audioDeviceLayer = layer;
    }*/

    WebRtc_UWord16 NumOfSendingChannels();

    // Convenience methods for calling statistics().SetLastError().
    void SetLastError(const WebRtc_Word32 error) const;
    void SetLastError(const WebRtc_Word32 error, const TraceLevel level) const;
    void SetLastError(const WebRtc_Word32 error, const TraceLevel level,
                      const char* msg) const;

protected:
    const WebRtc_UWord32 _instanceId;
    CriticalSectionWrapper* _apiCritPtr;
    Statistics _engineStatistics;
    AudioDeviceModule* _audioDevicePtr;
    ChannelManager _channelManager;    
    
    TransmitMixer* _transmitMixerPtr;

    OutputMixer* _outputMixerPtr;
    
    AudioProcessing* _audioProcessingModulePtr;
    ProcessThread* _moduleProcessThreadPtr;

    bool _externalRecording;
    bool _externalPlayout;

    //AudioDeviceModule::AudioLayer _audioDeviceLayer;

    MediaEngineExternalTransport _externalTransportPtr;

    SharedData();
    virtual ~SharedData();
};


} //  namespace webrtc
#endif // WEBRTC_VOICE_ENGINE_SHARED_DATA_H
