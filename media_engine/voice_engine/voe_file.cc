#include "voe_file.h"
#include "shared_data.h"
#include "channel.h"
#include "trace.h"
#include <assert.h>
#include "file_wrapper.h"
#include "transmit_mixer.h"
#include "output_mixer.h"

using namespace webrtc;

VoeFileImpl::VoeFileImpl(SharedData* shared) : _shared(shared) {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "VoeFileImpl::VoeFileImpl() - ctor");
}

VoeFileImpl::~VoeFileImpl() {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "VoeFileImpl::~VoeFileImpl() - dtor");
}

int VoeFileImpl::StartPlayingFileLocally(
  int channel,
  const char fileNameUTF8[1024],
  bool loop, FileFormats format,
  float volumeScaling,
  int startPointMs,
  int stopPointMs) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StartPlayingFileLocally(channel=%d, fileNameUTF8[]=%s, "
    "loop=%d, format=%d, volumeScaling=%5.3f, startPointMs=%d,"
    " stopPointMs=%d)",
    channel, fileNameUTF8, loop, format, volumeScaling,
    startPointMs, stopPointMs);
  assert(1024 == FileWrapper::kMaxFileNameSize);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "StartPlayingFileLocally() failed to locate channel");
    return -1;
  }

  return channelPtr->StartPlayingFileLocally(fileNameUTF8,
    loop,
    format,
    startPointMs,
    volumeScaling,
    stopPointMs,
    NULL);
}

int VoeFileImpl::StopPlayingFileLocally(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StopPlayingFileLocally()");
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "StopPlayingFileLocally() failed to locate channel");
    return -1;
  }
  return channelPtr->StopPlayingFileLocally();
}

int VoeFileImpl::IsPlayingFileLocally(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "IsPlayingFileLocally(channel=%d)", channel);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "StopPlayingFileLocally() failed to locate channel");
    return -1;
  }
  return channelPtr->IsPlayingFileLocally();
}

int VoeFileImpl::ScaleLocalFilePlayout(int channel, float scale) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "ScaleLocalFilePlayout(channel=%d, scale=%5.3f)",
    channel, scale);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "StopPlayingFileLocally() failed to locate channel");
    return -1;
  }
  return channelPtr->ScaleLocalFilePlayout(scale);
}

int VoeFileImpl::StartPlayingFileAsMicrophone(int channel,
  const char fileNameUTF8[1024],
  bool loop,
  bool mixWithMicrophone,
  FileFormats format,
  float volumeScaling) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StartPlayingFileAsMicrophone(channel=%d, fileNameUTF8=%s, "
    "loop=%d, mixWithMicrophone=%d, format=%d, "
    "volumeScaling=%5.3f)",
    channel, fileNameUTF8, loop, mixWithMicrophone, format,
    volumeScaling);
  assert(1024 == FileWrapper::kMaxFileNameSize);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  const WebRtc_UWord32 startPointMs(0);
  const WebRtc_UWord32 stopPointMs(0);

  if (channel == -1) {
    int res = _shared->transmit_mixer()->StartPlayingFileAsMicrophone(
      fileNameUTF8,
      loop,
      format,
      startPointMs,
      volumeScaling,
      stopPointMs,
      NULL);
    if (res) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
        VoEId(_shared->instance_id(), -1),
        "StartPlayingFileAsMicrophone() failed to start playing file");
      return(-1);
    } else {
      _shared->transmit_mixer()->SetMixWithMicStatus(mixWithMicrophone);
      return(0);
    }
  } else {
    // Add file after demultiplexing <=> affects one channel only
    voe::ScopedChannel sc(_shared->channel_manager(), channel);
    voe::Channel* channelPtr = sc.ChannelPtr();
    if (channelPtr == NULL) {
      _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
        "StartPlayingFileAsMicrophone() failed to locate channel");
      return -1;
    }

    int res = channelPtr->StartPlayingFileAsMicrophone(fileNameUTF8,
      loop,
      format,
      startPointMs,
      volumeScaling,
      stopPointMs,
      NULL);
    if (res) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
        VoEId(_shared->instance_id(), -1),
        "StartPlayingFileAsMicrophone() failed to start playing file");
      return -1;
    } else {
      channelPtr->SetMixWithMicStatus(mixWithMicrophone);
      return 0;
    }
  }
}

int VoeFileImpl::StopPlayingFileAsMicrophone(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StopPlayingFileAsMicrophone(channel=%d)", channel);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  if (channel == -1) {
    // Stop adding file before demultiplexing <=> affects all channels
    return _shared->transmit_mixer()->StopPlayingFileAsMicrophone();
  } else {
    // Stop adding file after demultiplexing <=> affects one channel only
    voe::ScopedChannel sc(_shared->channel_manager(), channel);
    voe::Channel* channelPtr = sc.ChannelPtr();
    if (channelPtr == NULL) {
      _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
        "StopPlayingFileAsMicrophone() failed to locate channel");
      return -1;
    }
    return channelPtr->StopPlayingFileAsMicrophone();
  }
}

int VoeFileImpl::IsPlayingFileAsMicrophone(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "IsPlayingFileAsMicrophone(channel=%d)", channel);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  if (channel == -1) {
    return _shared->transmit_mixer()->IsPlayingFileAsMicrophone();
  } else {
    // Stop adding file after demultiplexing <=> affects one channel only
    voe::ScopedChannel sc(_shared->channel_manager(), channel);
    voe::Channel* channelPtr = sc.ChannelPtr();
    if (channelPtr == NULL) {
      _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
        "IsPlayingFileAsMicrophone() failed to locate channel");
      return -1;
    }
    return channelPtr->IsPlayingFileAsMicrophone();
  }
}

int VoeFileImpl::ScaleFileAsMicrophonePlayout(int channel, float scale) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "ScaleFileAsMicrophonePlayout(channel=%d, scale=%5.3f)",
    channel, scale);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  if (channel == -1) {
    return _shared->transmit_mixer()->ScaleFileAsMicrophonePlayout(scale);
  } else {
    // Stop adding file after demultiplexing <=> affects one channel only
    voe::ScopedChannel sc(_shared->channel_manager(), channel);
    voe::Channel* channelPtr = sc.ChannelPtr();
    if (channelPtr == NULL) {
      _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
        "IsPlayingFileAsMicrophone() failed to locate channel");
      return -1;
    }
    return channelPtr->ScaleFileAsMicrophonePlayout(scale);
  }
}

int VoeFileImpl::StartRecordingPlayout(
  int channel, const char* fileNameUTF8, CodecInst* compression,
  int maxSizeBytes) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StartRecordingPlayout(channel=%d, fileNameUTF8=%s, "
    "compression, maxSizeBytes=%d)",
    channel, fileNameUTF8, maxSizeBytes);
  assert(1024 == FileWrapper::kMaxFileNameSize);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  if (channel == -1) {
    return _shared->output_mixer()->StartRecordingPlayout
      (fileNameUTF8, compression);
  } else {
    // Add file after demultiplexing <=> affects one channel only
    voe::ScopedChannel sc(_shared->channel_manager(), channel);
    voe::Channel* channelPtr = sc.ChannelPtr();
    if (channelPtr == NULL) {
      _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
        "StartRecordingPlayout() failed to locate channel");
      return -1;
    }
    return channelPtr->StartRecordingPlayout(fileNameUTF8, compression);
  }
}

int VoeFileImpl::StopRecordingPlayout(int channel) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StopRecordingPlayout(channel=%d)", channel);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  if (channel == -1) {
    return _shared->output_mixer()->StopRecordingPlayout();
  } else {
    voe::ScopedChannel sc(_shared->channel_manager(), channel);
    voe::Channel* channelPtr = sc.ChannelPtr();
    if (channelPtr == NULL) {
      _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
        "StopRecordingPlayout() failed to locate channel");
      return -1;
    }
    return channelPtr->StopRecordingPlayout();
  }
}

int VoeFileImpl::StartRecordingMicrophone(
  const char* fileNameUTF8, CodecInst* compression, int maxSizeBytes) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StartRecordingMicrophone(fileNameUTF8=%s, compression, "
    "maxSizeBytes=%d)", fileNameUTF8, maxSizeBytes);
  assert(1024 == FileWrapper::kMaxFileNameSize);

  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  if (_shared->transmit_mixer()->StartRecordingMicrophone(fileNameUTF8,
    compression)) {
    WEBRTC_TRACE(kTraceError, kTraceVoice,
      VoEId(_shared->instance_id(), -1),
      "StartRecordingMicrophone() failed to start recording");
    return -1;
  }
  if (_shared->audio_device()->Recording()) {
    return 0;
  }
  if (!_shared->ext_recording()) {
    if (_shared->audio_device()->InitRecording() != 0) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
        VoEId(_shared->instance_id(), -1),
        "StartRecordingMicrophone() failed to initialize recording");
      return -1;
    }
    if (_shared->audio_device()->StartRecording() != 0) {
      WEBRTC_TRACE(kTraceError, kTraceVoice,
        VoEId(_shared->instance_id(), -1),
        "StartRecordingMicrophone() failed to start recording");
      return -1;
    }
  }
  return 0;
}

int VoeFileImpl::StopRecordingMicrophone() {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StopRecordingMicrophone()");
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }

  int err = 0;

  // TODO(xians): consider removing Start/StopRecording() in
  // Start/StopRecordingMicrophone() if no channel is recording.
  if (_shared->NumOfSendingChannels() == 0 &&
    _shared->audio_device()->Recording()) {
    // Stop audio-device recording if no channel is recording
    if (_shared->audio_device()->StopRecording() != 0)
    {
      _shared->SetLastError(VE_CANNOT_STOP_RECORDING, kTraceError,
        "StopRecordingMicrophone() failed to stop recording");
      err = -1;
    }
  }

  if (_shared->transmit_mixer()->StopRecordingMicrophone() != 0) {
    WEBRTC_TRACE(kTraceError, kTraceVoice,
      VoEId(_shared->instance_id(), -1),
      "StopRecordingMicrophone() failed to stop recording to mixer");
    err = -1;
  }

  return err;
}
