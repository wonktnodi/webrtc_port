#include "voe_codec.h"
#include "trace.h"
#include "shared_data.h"
#include "audio_coding_module.h"
#include "channel.h"

using namespace webrtc;

VoECodecImpl::VoECodecImpl(SharedData* shared) : _shared(shared) {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "VoECodecImpl() - ctor");
}

VoECodecImpl::~VoECodecImpl() {
  WEBRTC_TRACE(kTraceMemory, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "~VoECodecImpl() - dtor");
}


int VoECodecImpl::NumOfCodecs() {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "NumOfCodecs()");

  // Number of supported codecs in the ACM
  WebRtc_UWord8 nSupportedCodecs = AudioCodingModule::NumberOfCodecs();

  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
    VoEId(_shared->instance_id(), -1),
    "NumOfCodecs() => %u", nSupportedCodecs);
  return (nSupportedCodecs);
}

int VoECodecImpl::GetCodec(int index, CodecInst& codec) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "GetCodec(index=%d, codec=?)", index);
  CodecInst acmCodec;
  if (AudioCodingModule::Codec(index, (CodecInst&) acmCodec) == -1) {
    _shared->SetLastError(VE_INVALID_LISTNR, kTraceError,
      "GetCodec() invalid index");
    return -1;
  }

  ACMToExternalCodecRepresentation(codec, acmCodec);
  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
    VoEId(_shared->instance_id(), -1),
    "GetCodec() => plname=%s, pacsize=%d, plfreq=%d, pltype=%d, "
    "channels=%d, rate=%d", codec.plname, codec.pacsize,
    codec.plfreq, codec.pltype, codec.channels, codec.rate);  
  return 0;
}

int VoECodecImpl::GetCodecByPayloadType(uint8_t payload_type, CodecInst& codec) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "GetCodecByPayloadType(pt=%d)", payload_type);
  CodecInst acmCodec;
  for(uint32_t index = 0; index < AudioCodingModule::NumberOfCodecs(); ++ index) {
    if (AudioCodingModule::Codec(index, (CodecInst&) acmCodec) == -1) {
      _shared->SetLastError(VE_INVALID_LISTNR, kTraceError,
        "GetCodec() invalid index");
      return -1;
    }
    if (acmCodec.pltype == payload_type) {
      ACMToExternalCodecRepresentation(codec, acmCodec);
      WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
        VoEId(_shared->instance_id(), -1),
        "GetCodec() => plname=%s, pacsize=%d, plfreq=%d, pltype=%d, "
        "channels=%d, rate=%d", codec.plname, codec.pacsize,
        codec.plfreq, codec.pltype, codec.channels, codec.rate);  
      return 0;
    }
  }  
  return -1;
}

int VoECodecImpl::SetSendCodec(int channel, const CodecInst& codec) {
  CodecInst copyCodec;
  ExternalToACMCodecRepresentation(copyCodec, codec);

  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "SetSendCodec(channel=%d, codec)", channel);
  WEBRTC_TRACE(kTraceInfo, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "codec: plname=%s, pacsize=%d, plfreq=%d, pltype=%d, "
    "channels=%d, rate=%d", codec.plname, codec.pacsize,
    codec.plfreq, codec.pltype, codec.channels, codec.rate);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  // External sanity checks performed outside the ACM
  if ((STR_CASE_CMP(copyCodec.plname, "L16") == 0) &&
    (copyCodec.pacsize >= 960)) {
    _shared->SetLastError(VE_INVALID_ARGUMENT, kTraceError,
      "SetSendCodec() invalid L16 packet size");
    return -1;
  }
  if (!STR_CASE_CMP(copyCodec.plname, "CN")
    || !STR_CASE_CMP(copyCodec.plname, "TELEPHONE-EVENT")
    || !STR_CASE_CMP(copyCodec.plname, "RED")) {
    _shared->SetLastError(VE_INVALID_ARGUMENT, kTraceError,
      "SetSendCodec() invalid codec name");
    return -1;
  }
  if ((copyCodec.channels != 1) && (copyCodec.channels != 2)) {
    _shared->SetLastError(VE_INVALID_ARGUMENT, kTraceError,
      "SetSendCodec() invalid number of channels");
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "GetSendCodec() failed to locate channel");
    return -1;
  }
  if (!AudioCodingModule::IsCodecValid((CodecInst&) copyCodec)) {
    _shared->SetLastError(VE_INVALID_ARGUMENT, kTraceError,
      "SetSendCodec() invalid codec");
    return -1;
  }

  if (channelPtr->SetSendCodec(copyCodec) != 0) {
    _shared->SetLastError(VE_CANNOT_SET_SEND_CODEC, kTraceError,
      "SetSendCodec() failed to set send codec");
    return -1;
  }

  return 0;
}

int VoECodecImpl::GetSendCodec(int channel, CodecInst& codec) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "GetSendCodec(channel=%d, codec=?)", channel);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "GetSendCodec() failed to locate channel");
    return -1;
  }
  CodecInst acmCodec;
  if (channelPtr->GetSendCodec(acmCodec) != 0) {
    _shared->SetLastError(VE_CANNOT_GET_SEND_CODEC, kTraceError,
      "GetSendCodec() failed to get send codec");
    return -1;
  }
  ACMToExternalCodecRepresentation(codec, acmCodec);
  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
    VoEId(_shared->instance_id(), -1),
    "GetSendCodec() => plname=%s, pacsize=%d, plfreq=%d, "
    "channels=%d, rate=%d", codec.plname, codec.pacsize,
    codec.plfreq, codec.channels, codec.rate);
  return 0;
}

int VoECodecImpl::GetRecCodec(int channel, CodecInst& codec) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice, VoEId(_shared->instance_id(), -1),
    "GetRecCodec(channel=%d, codec=?)", channel);
  if (!_shared->statistics().Initialized()) {
    _shared->SetLastError(VE_NOT_INITED, kTraceError);
    return -1;
  }
  voe::ScopedChannel sc(_shared->channel_manager(), channel);
  voe::Channel* channelPtr = sc.ChannelPtr();
  if (channelPtr == NULL) {
    _shared->SetLastError(VE_CHANNEL_NOT_VALID, kTraceError,
      "GetRecCodec() failed to locate channel");
    return -1;
  }
  CodecInst acmCodec;
  if (channelPtr->GetRecCodec(acmCodec) != 0) {
    _shared->SetLastError(VE_CANNOT_GET_REC_CODEC, kTraceError,
      "GetRecCodec() failed to get received codec");
    return -1;
  }
  ACMToExternalCodecRepresentation(codec, acmCodec);
  WEBRTC_TRACE(kTraceStateInfo, kTraceVoice,
    VoEId(_shared->instance_id(), -1),
    "GetRecCodec() => plname=%s, pacsize=%d, plfreq=%d, "
    "channels=%d, rate=%d", codec.plname, codec.pacsize,
    codec.plfreq, codec.channels, codec.rate);
  return 0;
}


void VoECodecImpl::ACMToExternalCodecRepresentation(CodecInst& toInst,
  const CodecInst& fromInst) {
  toInst = fromInst;
  if (STR_CASE_CMP(fromInst.plname,"SILK") == 0) {
    if (fromInst.plfreq == 12000) {
      if (fromInst.pacsize == 320) {
        toInst.pacsize = 240;
      } else if (fromInst.pacsize == 640) {
        toInst.pacsize = 480;
      } else if (fromInst.pacsize == 960) {
        toInst.pacsize = 720;
      }
    } else if (fromInst.plfreq == 24000) {
      if (fromInst.pacsize == 640) {
        toInst.pacsize = 480;
      } else if (fromInst.pacsize == 1280) {
        toInst.pacsize = 960;
      } else if (fromInst.pacsize == 1920) {
        toInst.pacsize = 1440;
      }
    }
  }
}

void VoECodecImpl::ExternalToACMCodecRepresentation(CodecInst& toInst,
  const CodecInst& fromInst) {
  toInst = fromInst;
  if (STR_CASE_CMP(fromInst.plname,"SILK") == 0) {
    if (fromInst.plfreq == 12000) {
      if (fromInst.pacsize == 240) {
        toInst.pacsize = 320;
      } else if (fromInst.pacsize == 480) {
        toInst.pacsize = 640;
      } else if (fromInst.pacsize == 720) {
        toInst.pacsize = 960;
      }
    } else if (fromInst.plfreq == 24000) {
      if (fromInst.pacsize == 480) {
        toInst.pacsize = 640;
      } else if (fromInst.pacsize == 960) {
        toInst.pacsize = 1280;
      } else if (fromInst.pacsize == 1440) {
        toInst.pacsize = 1920;
      }
    }
  }
}
