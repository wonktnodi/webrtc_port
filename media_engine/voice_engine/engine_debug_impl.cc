#include "engine_debug_impl.h"
#include "shared_data.h"
#include "trace.h"
#include "channel.h"
#include "file_wrapper.h"

MediaEngineDebugImpl::MediaEngineDebugImpl(SharedData* shared)
  : _shared(shared) {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
      "MediaEngineDebugImpl::MediaEngineDebugImpl() - ctor");
}

MediaEngineDebugImpl::~MediaEngineDebugImpl() {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "MediaEngineDebugImpl::~MediaEngineDebugImpl() - dtor");
}

int MediaEngineDebugImpl::StartRTPDump(int channel, const char fileNameUTF8[1024], RTPDirections direction) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StartRTPDump(channel=%d, fileNameUTF8=%s, direction=%d)",
    channel, fileNameUTF8, direction);
  assert(1024 == FileWrapper::kMaxFileNameSize);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "StartRTPDump() failed to locate channel");
    return -1;
  }
  return channelPtr->StartRTPDump(fileNameUTF8, direction);
}

int MediaEngineDebugImpl::StopRTPDump(int channel, RTPDirections direction) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "StopRTPDump(channel=%d, direction=%d)", channel, direction);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "StopRTPDump() failed to locate channel");
    return -1;
  }
  return channelPtr->StopRTPDump(direction);
}

int MediaEngineDebugImpl::RTPDumpIsActive(int channel, RTPDirections direction) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "RTPDumpIsActive(channel=%d, direction=%d)",
    channel, direction);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "StopRTPDump() failed to locate channel");
    return -1;
  }
  return channelPtr->RTPDumpIsActive(direction);
}