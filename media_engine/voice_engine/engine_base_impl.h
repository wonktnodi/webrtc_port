#ifndef __MEDIA_ENGINE_BASE_IMPL_H__
#define __MEDIA_ENGINE_BASE_IMPL_H__

#include <assert.h>
#include "module_common_types.h"
#include "atomic32.h"
#include "shared_data.h"

using namespace webrtc;
class MediaEngineImpl : public webrtc::SharedData,
  public AudioTransport, public AudioDeviceObserver {
public:
  MediaEngineImpl() : _oldVoEMicLevel(0), _oldMicLevel(0),
    _ref_count(0) {
  }

  virtual ~MediaEngineImpl() {
    assert(_ref_count.Value() == 0);
  }

  int AddRef();

  // This implements the Release() method for all the inherited interfaces.
  virtual int Release();


  virtual int Init();

  virtual int Terminate();

  // AudioTransport
  virtual WebRtc_Word32
    RecordedDataIsAvailable(const void* audioSamples,
    const WebRtc_UWord32 nSamples,
    const WebRtc_UWord8 nBytesPerSample,
    const WebRtc_UWord8 nChannels,
    const WebRtc_UWord32 samplesPerSec,
    const WebRtc_UWord32 totalDelayMS,
    const WebRtc_Word32 clockDrift,
    const WebRtc_UWord32 currentMicLevel,
    WebRtc_UWord32& newMicLevel);

  virtual WebRtc_Word32 NeedMorePlayData(const WebRtc_UWord32 nSamples,
    const WebRtc_UWord8 nBytesPerSample,
    const WebRtc_UWord8 nChannels,
    const WebRtc_UWord32 samplesPerSec,
    void* audioSamples,
    WebRtc_UWord32& nSamplesOut);
  // AudioDeviceObserver
  virtual void OnErrorIsReported(const ErrorCode error);
  virtual void OnWarningIsReported(const WarningCode warning);

private:
  WebRtc_Word32 TerminateInternal();
private:
  WebRtc_UWord32 _oldVoEMicLevel;
  WebRtc_UWord32 _oldMicLevel;
  AudioFrame _audioFrame;
  Atomic32 _ref_count;
};

#endif // __MEDIA_ENGINE_BASE_IMPL_H__
