#include "voe_base.h"
#include "trace.h"
#include "voice_engine_defines.h"
#include "shared_data.h"
#include "critical_section_wrapper.h"
#include "channel.h"
#include "transmit_mixer.h"

VoEBaseImpl::VoEBaseImpl(SharedData* shared) : _shared(shared) {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "VoEBaseImpl() - ctor");
}

VoEBaseImpl::~VoEBaseImpl() {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "~VoEBaseImpl() - dtor");

  TerminateInternal();
}


int VoEBaseImpl::MaxNumOfChannels() {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "MaxNumOfChannels()");
  /*WebRtc_Word32 maxNumOfChannels =
    _shared->channel_manager().MaxNumOfChannels();
  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
    VoEId(_shared->instance_id(), -1),
    "MaxNumOfChannels() => %d", maxNumOfChannels);
  return (maxNumOfChannels);*/
  return 0;
}

int VoEBaseImpl::CreateChannel() {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "CreateChannel()");

  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  WebRtc_Word32 channelId = -1;

  if (!_shared->channel_manager().CreateChannel(channelId)) {
    _shared->SetLastError(VE_CHANNEL_NOT_CREATED, kTraceError,
                          "CreateChannel() failed to allocate memory for channel");
    return -1;
  }

  bool destroyChannel(false);
  {
    voe::ScopedChannel sc(_shared->channel_manager(), channelId);
    voe::Channel* channelPtr = sc.ChannelPtr();

    if (channelPtr == NULL) {
      _shared->SetLastError(VE_CHANNEL_NOT_CREATED, kTraceError,
                            "CreateChannel() failed to allocate memory for channel");
      return -1;
    } else if (channelPtr->SetEngineInformation(_shared->statistics(),
               *_shared->output_mixer(),
               *_shared->transmit_mixer(),
               *_shared->process_thread(),
               *_shared->audio_device(),
               /*_voiceEngineObserverPtr*/0,
               /*&_callbackCritSect*/0) != 0) {
      destroyChannel = true;
      _shared->SetLastError(VE_CHANNEL_NOT_CREATED, kTraceError,
                            "CreateChannel() failed to associate engine and channel."
                            " Destroying channel.");
    } else if (channelPtr->Init() != 0) {
      destroyChannel = true;
      _shared->SetLastError(VE_CHANNEL_NOT_CREATED, kTraceError,
                            "CreateChannel() failed to initialize channel. Destroying"
                            " channel.");
    }

    if (_shared->external_transport() &&
        channelPtr->RegisterExternalTransport(*_shared->external_transport())) {
      destroyChannel = true;
      _shared->SetLastError(VE_CHANNEL_NOT_CREATED, kTraceError,
                            "CreateChannel() faled to register external transport. Destroying"
                            " channel.");
    }
  }

  if (destroyChannel) {
    _shared->channel_manager().DestroyChannel(channelId);
    return -1;
  }

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
               VoEId(_shared->instance_id(), -1),
               "CreateChannel() => %d", channelId);

  return channelId;
}

int VoEBaseImpl::DeleteChannel(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "DeleteChannel(channel=%d)", channel);
  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  {
    voe::ScopedChannel sc(_shared->channel_manager(), channel);
    voe::Channel* channelPtr = sc.ChannelPtr();

    if (channelPtr == NULL) {
      _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                            "DeleteChannel() failed to locate channel");
      return -1;
    }
  }

  StopPlayout(channel);
  StopSend(channel);
  if (_shared->channel_manager().DestroyChannel(channel) != 0) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "DeleteChannel() failed to destroy channel");
    return -1;
  }

  if (StopSend() != 0) {
    return -1;
  }

  if (StopPlayout() != 0) {
    return -1;
  }

  return 0;
}

int VoEBaseImpl::StartSend(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "StartSend(channel=%d)", channel);

  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "StartSend() failed to locate channel");
    return -1;
  }

  if (channelPtr->Sending()) {
    return 0;
  }

//#ifndef WEBRTC_EXTERNAL_TRANSPORT
//  if (!channelPtr->ExternalTransport()
//    && !channelPtr->SendSocketsInitialized())
//  {
//    _shared->SetLastError(VE_DESTINATION_NOT_INITED, kTraceError,
//      "StartSend() must set send destination first");
//    return -1;
//  }
//#endif
  if (StartSend() != 0) {
    _shared->SetLastError(VE_AUDIO_DEVICE_MODULE_ERROR, kTraceError,
                          "StartSend() failed to start recording");
    return -1;
  }

  return channelPtr->StartSend();
}

int VoEBaseImpl::StopSend(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "StopSend(channel=%d)", channel);
  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "StopSend() failed to locate channel");
    return -1;
  }

  if (channelPtr->StopSend() != 0) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_shared->instance_id(), -1),
                 "StopSend() failed to stop sending for channel %d", channel);
  }

  return StopSend();
}

int VoEBaseImpl::SendingStatus(int channel, bool &sending) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "Sending(channel=%d)", channel);
  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "StopSend() failed to locate channel");
    return -1;
  }

  sending = channelPtr->Sending();
  return 0;
}

int VoEBaseImpl::StartPlayout(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "StartPlayout(channel=%d)", channel);
  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "StartPlayout() failed to locate channel");
    return -1;
  }

  if (channelPtr->Playing()) {
    return 0;
  }

  if (StartPlayout() != 0) {
    _shared->SetLastError(VE_AUDIO_DEVICE_MODULE_ERROR, kTraceError,
                          "StartPlayout() failed to start playout");
    return -1;
  }

  return channelPtr->StartPlayout();
}

int VoEBaseImpl::StopPlayout(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "StopPlayout(channel=%d)", channel);
  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "StopPlayout() failed to locate channel");
    return -1;
  }

  if (channelPtr->StopPlayout() != 0) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice,
                 VoEId(_shared->instance_id(), -1),
                 "StopPlayout() failed to stop playout for channel %d", channel);
  }

  return StopPlayout();
}

int VoEBaseImpl::StartReceive(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "StartReceive(channel=%d)", channel);
  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "StartReceive() failed to locate channel");
    return -1;
  }

  return channelPtr->StartReceiving();
}

int VoEBaseImpl::StopReceive(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "StopListen(channel=%d)", channel);
  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "SetLocalReceiver() failed to locate channel");
    return -1;
  }

  return channelPtr->StopReceiving();
}

int VoEBaseImpl::SetNetEQBGNMode(int channel, NetEqBgnModes mode) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "SetNetEQBGNMode(channel=%i, mode=%i)", channel, mode);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "SetNetEQBGNMode() failed to locate channel");
    return -1;
  }

  return channelPtr->SetNetEQBGNMode(mode);
}

int VoEBaseImpl::GetNetEQBGNMode(int channel, NetEqBgnModes& mode) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "GetNetEQBGNMode(channel=%i, mode=?)", channel);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "GetNetEQBGNMode() failed to locate channel");
    return -1;
  }

  return channelPtr->GetNetEQBGNMode(mode);
}

int VoEBaseImpl::SetNetEQPlayoutMode(int channel, NetEqModes mode) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "SetNetEQPlayoutMode(channel=%i, mode=%i)", channel, mode);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "SetNetEQPlayoutMode() failed to locate channel");
    return -1;
  }

  return channelPtr->SetNetEQPlayoutMode(mode);
}

int VoEBaseImpl::GetNetEQPlayoutMode(int channel, NetEqModes& mode) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "GetNetEQPlayoutMode(channel=%i, mode=?)", channel);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "GetNetEQPlayoutMode() failed to locate channel");
    return -1;
  }

  return channelPtr->GetNetEQPlayoutMode(mode);
}

int VoEBaseImpl::GetOnHoldStatus(int channel, bool& enabled,
                                 OnHoldModes& mode) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "GetOnHoldStatus(channel=%d, enabled=?, mode=?)", channel);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "GetOnHoldStatus() failed to locate channel");
    return -1;
  }

  return channelPtr->GetOnHoldStatus(enabled, mode);
}

int VoEBaseImpl::SetOnHoldStatus(int channel, bool enable,
                                 OnHoldModes mode /* = kHoldSendAndPlay */) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "SetOnHoldStatus(channel=%d, enable=%d, mode=%d)", channel,
               enable, mode);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "SetOnHoldStatus() failed to locate channel");
    return -1;
  }

  return channelPtr->SetOnHoldStatus(enable, mode);
}

int VoEBaseImpl::GetDelayWithPlayoutTimeStamp(int channel,
    WebRtc_UWord32 timestamp, WebRtc_UWord32 &delay) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "GetDelayWithPlayoutTimeStamp(channel=%d)", channel);
  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "SetLocalReceiver() failed to locate channel");
    return -1;
  }

  return channelPtr->GetDelayWithPlayoutTimeStamp(timestamp, delay);
}

int VoEBaseImpl::GetNetworkStatistics(int channel, NetworkStatistics& stats) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "GetNetworkStatistics(channel=%d)", channel);
  CriticalSectionScoped cs(_shared->crit_sec());

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
                          "SetLocalReceiver() failed to locate channel");
    return -1;
  }

  return channelPtr->GetNetworkStatistics(stats);
}

int VoEBaseImpl::GetVersion(char version[1024]) {
  return 0;
}

int VoEBaseImpl::LastError() {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "LastError()");
  return (_shared->statistics().LastError());
}

WebRtc_Word32 VoEBaseImpl::StartPlayout() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "VoEBaseImpl::StartPlayout()");

  if (_shared->audio_device()->Playing()) {
    return 0;
  }

  if (!_shared->ext_playout()) {
    if (_shared->audio_device()->InitPlayout() != 0) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
                   VoEId(_shared->instance_id(), -1),
                   "StartPlayout() failed to initialize playout");
      return -1;
    }

    if (_shared->audio_device()->StartPlayout() != 0) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
                   VoEId(_shared->instance_id(), -1),
                   "StartPlayout() failed to start playout");
      return -1;
    }
  }

  return 0;
}

WebRtc_Word32 VoEBaseImpl::StopPlayout() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "VoEBaseImpl::StopPlayout()");

  WebRtc_Word32 numOfChannels = _shared->channel_manager().NumOfChannels();

  if (numOfChannels <= 0) {
    return 0;
  }

  WebRtc_UWord16 nChannelsPlaying(0);
  WebRtc_Word32* channelsArray = new WebRtc_Word32[numOfChannels];

  // Get number of playing channels
  _shared->channel_manager().GetChannelIds(channelsArray, numOfChannels);

  for (int i = 0; i < numOfChannels; i++) {
    voe::ScopedChannel sc(_shared->channel_manager(), channelsArray[i]);
    voe::Channel* chPtr = sc.ChannelPtr();

    if (chPtr) {
      if (chPtr->Playing()) {
        nChannelsPlaying++;
      }
    }
  }

  delete[] channelsArray;

  // Stop audio-device playing if no channel is playing out
  if (nChannelsPlaying == 0) {
    if (_shared->audio_device()->StopPlayout() != 0) {
      _shared->SetLastError(VE_CANNOT_STOP_PLAYOUT, kTraceError,
                            "StopPlayout() failed to stop playout");
      return -1;
    }
  }

  return 0;
}

WebRtc_Word32 VoEBaseImpl::StartSend() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "VoEBaseImpl::StartSend()");

  if (_shared->audio_device()->Recording()) {
    return 0;
  }

  if (!_shared->ext_recording()) {
    if (_shared->audio_device()->InitRecording() != 0) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
                   VoEId(_shared->instance_id(), -1),
                   "StartSend() failed to initialize recording");
      return -1;
    }

    if (_shared->audio_device()->StartRecording() != 0) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
                   VoEId(_shared->instance_id(), -1),
                   "StartSend() failed to start recording");
      return -1;
    }
  }

  return 0;
}

WebRtc_Word32 VoEBaseImpl::StopSend() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "VoEBaseImpl::StopSend()");

  if (_shared->NumOfSendingChannels() == 0 &&
      !_shared->transmit_mixer()->IsRecordingMic()) {
    // Stop audio-device recording if no channel is recording
    if (_shared->audio_device()->StopRecording() != 0) {
      _shared->SetLastError(VE_CANNOT_STOP_RECORDING, kTraceError,
                            "StopSend() failed to stop recording");
      return -1;
    }

    _shared->transmit_mixer()->StopSend();
  }

  return 0;
}

WebRtc_Word32 VoEBaseImpl::TerminateInternal() {
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_shared->instance_id(), -1),
               "VoEBaseImpl::TerminateInternal()");

  // Delete any remaining channel objects
  WebRtc_Word32 numOfChannels = _shared->channel_manager().NumOfChannels();

  if (numOfChannels > 0) {
    WebRtc_Word32* channelsArray = new WebRtc_Word32[numOfChannels];
    _shared->channel_manager().GetChannelIds(channelsArray, numOfChannels);

    for (int i = 0; i < numOfChannels; i++) {
      DeleteChannel(channelsArray[i]);
    }

    delete[] channelsArray;
  }

  return 0;
}

int VoEBaseImpl::EnablePacketCombine(int channel, bool enable, uint16_t max_packet) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "EnablePacketCombine(channel=%i, mode=%i, packet=%i)", channel, enable, max_packet);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "SetNetEQBGNMode() failed to locate channel");
    return -1;
  }

  channelPtr->EnablePacketCombine(enable, max_packet);
  return 0;
}

int VoEBaseImpl::EnablePacketSeparate(int channel, bool enable) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "EnablePacketSeparate(channel=%i, mode=%i)", channel, enable);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();

  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "SetNetEQBGNMode() failed to locate channel");
    return -1;
  }

  channelPtr->EnablePacketSeparate(enable);
  return 0;
}
