#ifndef __MEDIA_ENGINE_VOE_CODEC_HPP__
#define __MEDIA_ENGINE_VOE_CODEC_HPP__

#include "engine_base.hpp"
#include "engine_voe_codec.h"

class MediaEngineVoeCodec {
public:
  // Factory for the VoEHardware sub-API. Increases an internal
  // reference counter if successful. Returns NULL if the API is not
  // supported or if construction fails.
  static MediaEngineVoeCodec* GetInterface(MediaEngine* engine) {
    MediaEngineVoeCodec *obj = 0;
    obj = new MediaEngineVoeCodec();

    if (0 == obj) return 0;

    int ret = media_engine_get_voe_codec(engine->_eng_ptr, obj->_obj_ptr);

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
  static int Release(MediaEngineVoeCodec *codec) {
    assert(codec);
    media_engine_release_voe_codec(codec->_obj_ptr);
    delete codec;
    return 0;
  }

  // Gets the number of supported codecs.
  virtual int NumOfCodecs() {
    assert(_obj_ptr);
    return media_engine_voe_codec_num_of_codecs(_obj_ptr);
  }

  // Get the |codec| information for a specified list |index|.
  virtual int GetCodec(int index, CodecInst& codec) {
    assert(_obj_ptr);
    return media_engine_voe_codec_get_codec(_obj_ptr, index, codec);
  }

  virtual int GetCodecByPayloadType(uint8_t payload_type, CodecInst& codec) {
    assert(_obj_ptr);
    return media_engine_voe_codec_get_codec_by_payload_type(_obj_ptr,
           payload_type, codec);
  }

  // Sets the |codec| for the |channel| to be used for sending.
  virtual int SetSendCodec(int channel, const CodecInst& codec) {
    assert(_obj_ptr);
    return media_engine_voe_codec_set_send_codec(_obj_ptr, channel, codec);
  }

  // Gets the |codec| parameters for the sending codec on a specified
  // |channel|.
  virtual int GetSendCodec(int channel, CodecInst& codec) {
    assert(_obj_ptr);
    return media_engine_voe_codec_get_send_codec(_obj_ptr, channel, codec);
  }

  // Gets the currently received |codec| for a specific |channel|.
  virtual int GetRecCodec(int channel, CodecInst& codec) {
    assert(_obj_ptr);
    return media_engine_voe_codec_get_rec_codec(_obj_ptr, channel, codec);
  }

  //// Sets the dynamic payload type number for a particular |codec| or
  //// disables (ignores) a codec for receiving. For instance, when receiving
  //// an invite from a SIP-based client, this function can be used to change
  //// the dynamic payload type number to match that in the INVITE SDP-
  //// message. The utilized parameters in the |codec| structure are:
  //// plname, plfreq, pltype and channels.
  //virtual int SetRecPayloadType(int channel, const CodecInst& codec) {
  //  return 0;
  //}

  //// Gets the actual payload type that is set for receiving a |codec| on a
  //// |channel|. The value it retrieves will either be the default payload
  //// type, or a value earlier set with SetRecPayloadType().
  //virtual int GetRecPayloadType(int channel, CodecInst& codec) {
  //  return 0;
  //}

  //// Sets the VAD/DTX (silence suppression) status and |mode| for a
  //// specified |channel|. Disabling VAD (through |enable|) will also disable
  //// DTX; it is not necessary to explictly set |disableDTX| in this case.
  //virtual int SetVADStatus(int channel, bool enable,
  //  VadModes mode = kVadConventional,
  //  bool disableDTX = false) {
  //  return 0;
  //}

  //// Gets the VAD/DTX status and |mode| for a specified |channel|.
  //virtual int GetVADStatus(int channel, bool& enabled, VadModes& mode,
  //  bool& disabledDTX) {
  //  return 0;
  //}

private:
  media_engine_voice_codec *_obj_ptr;
};

#endif // __MEDIA_ENGINE_VOE_CODEC_HPP__
