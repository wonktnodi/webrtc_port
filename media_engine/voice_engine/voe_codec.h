#ifndef __MEDIA_ENGINE_VOE_CODEC_IMPL_H__
#define __MEDIA_ENGINE_VOE_CODEC_IMPL_H__

#include "common_types.h"

namespace webrtc {
  class SharedData;
}

using namespace webrtc;

class VoECodecImpl {
public:  
  virtual int NumOfCodecs();

  virtual int GetCodec(int index, CodecInst& codec);
  virtual int GetCodecByPayloadType(uint8_t payload_type, CodecInst& codec);

  virtual int SetSendCodec(int channel, const CodecInst& codec);

  virtual int GetSendCodec(int channel, CodecInst& codec);

  virtual int GetRecCodec(int channel, CodecInst& codec);

public:
  VoECodecImpl(SharedData* shared);
  virtual ~VoECodecImpl();

private:
  void ACMToExternalCodecRepresentation(CodecInst& toInst,
    const CodecInst& fromInst);

  void ExternalToACMCodecRepresentation(CodecInst& toInst,
    const CodecInst& fromInst);

  SharedData* _shared;
};


#endif // __MEDIA_ENGINE_VOE_CODEC_IMPL_H__
