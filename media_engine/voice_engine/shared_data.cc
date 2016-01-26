/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "shared_data.h"

#include "critical_section_wrapper.h"
#include "trace.h"
#include "channel.h"
#include "transmit_mixer.h"
#include "output_mixer.h"
#include "engine_network_impl.h"

namespace webrtc {

static WebRtc_Word32 _gInstanceCounter = 0;

SharedData::SharedData()
  : _instanceId(++_gInstanceCounter),
    _apiCritPtr(CriticalSectionWrapper::CreateCriticalSection()),
    _engineStatistics(_gInstanceCounter),
    _audioDevicePtr(NULL),
    _channelManager(_gInstanceCounter),        
    _audioProcessingModulePtr(NULL),
    _moduleProcessThreadPtr(ProcessThread::CreateProcessThread()),
    _externalRecording(false),
    _externalPlayout(false),
    _externalTransportPtr(0) {
    Trace::CreateTrace();
    Trace::SetLevelFilter(WEBRTC_VOICE_ENGINE_DEFAULT_TRACE_FILTER, kTraceModuleAll);
    if (TransmitMixer::Create(_transmitMixerPtr, _gInstanceCounter) == 0) {
      _transmitMixerPtr->SetEngineInformation(*_moduleProcessThreadPtr,
        _engineStatistics,
        _channelManager);
    }

    if (OutputMixer::Create(_outputMixerPtr, _gInstanceCounter) == 0) {
        _outputMixerPtr->SetEngineInformation(_engineStatistics);
    }
   
    /*_audioDeviceLayer = AudioDeviceModule::kPlatformDefaultAudio;*/
}

SharedData::~SharedData() {
  OutputMixer::Destroy(_outputMixerPtr);
  TransmitMixer::Destroy(_transmitMixerPtr);
  
  
  AudioProcessing::Destroy(_audioProcessingModulePtr);
  if (_audioDevicePtr) {
    _audioDevicePtr->Release();
  }
  
  WebRtc_Word32 chl_arr[32] = {0}, chl_cnt = 32;
  _channelManager.GetChannelIds(chl_arr, chl_cnt);

  for(int i = 0; i < chl_cnt; ++ i) {
    _channelManager.DestroyChannel(chl_arr[i]);
  }

  delete _apiCritPtr;
  ProcessThread::DestroyProcessThread(_moduleProcessThreadPtr);


  Trace::ReturnTrace();
}

void SharedData::set_audio_device(AudioDeviceModule* audio_device) {
  // AddRef first in case the pointers are equal.
  if (audio_device)
    audio_device->AddRef();
  if (_audioDevicePtr)
    _audioDevicePtr->Release();
  _audioDevicePtr = audio_device;
}

void SharedData::set_external_transport(engine_transport *transport) {
  _externalTransportPtr.SetTransport(transport);
}

WebRtc_UWord16 SharedData::NumOfSendingChannels() {
  WebRtc_Word32 numOfChannels = _channelManager.NumOfChannels();
  if (numOfChannels <= 0) {
    return 0;
  }

  WebRtc_UWord16 nChannelsSending(0);
  WebRtc_Word32* channelsArray = new WebRtc_Word32[numOfChannels];

  _channelManager.GetChannelIds(channelsArray, numOfChannels);
  for (int i = 0; i < numOfChannels; i++) {
    voe::ScopedChannel sc(_channelManager, channelsArray[i]);
    Channel* chPtr = sc.ChannelPtr();
    if (chPtr) {
      if (chPtr->Sending()) {
        nChannelsSending++;
      }
    }
  }
  delete [] channelsArray;
  return nChannelsSending;
}

void SharedData::set_audio_processing(AudioProcessing* audio_processing) {
    if (_audioProcessingModulePtr)
      AudioProcessing::Destroy(_audioProcessingModulePtr);
    _audioProcessingModulePtr = audio_processing;
}
//
//WebRtc_UWord16 SharedData::NumOfSendingChannels()
//{
//    WebRtc_Word32 numOfChannels = _channelManager.NumOfChannels();
//    if (numOfChannels <= 0)
//    {
//        return 0;
//    }
//
//    WebRtc_UWord16 nChannelsSending(0);
//    WebRtc_Word32* channelsArray = new WebRtc_Word32[numOfChannels];
//
//    _channelManager.GetChannelIds(channelsArray, numOfChannels);
//    for (int i = 0; i < numOfChannels; i++)
//    {
//        voe::ScopedChannel sc(_channelManager, channelsArray[i]);
//        Channel* chPtr = sc.ChannelPtr();
//        if (chPtr)
//        {
//            if (chPtr->Sending())
//            {
//                nChannelsSending++;
//            }
//        }
//    }
//    delete [] channelsArray;
//    return nChannelsSending;
//}

void SharedData::SetLastError(const WebRtc_Word32 error) const {
  _engineStatistics.SetLastError(error);
}

void SharedData::SetLastError(const WebRtc_Word32 error,
                              const TraceLevel level) const {
  _engineStatistics.SetLastError(error, level);
}

void SharedData::SetLastError(const WebRtc_Word32 error, const TraceLevel level,
                              const char* msg) const {
  _engineStatistics.SetLastError(error, level, msg);
}



}  //  namespace webrtc
