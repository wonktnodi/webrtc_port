#ifndef __MEDIA_ENGINE_VOE_CODEC_H__
#define __MEDIA_ENGINE_VOE_CODEC_H__

#include "common_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct media_engine_voice_codec media_engine_voice_codec;

int media_engine_voe_codec_num_of_codecs(media_engine_voice_codec*);

int media_engine_voe_codec_get_codec(media_engine_voice_codec*, int,
                                     webrtc::CodecInst&);
int media_engine_voe_codec_get_codec_by_payload_type(media_engine_voice_codec*,
    uint8_t payload_type, webrtc::CodecInst&);

int media_engine_voe_codec_set_send_codec(media_engine_voice_codec*, int,
    const webrtc::CodecInst&);

int media_engine_voe_codec_get_send_codec(media_engine_voice_codec*,
    int, webrtc::CodecInst&);

int media_engine_voe_codec_get_rec_codec(media_engine_voice_codec*,
    int, webrtc::CodecInst&);

#ifdef __cplusplus
}
#endif

#endif // __MEDIA_ENGINE_VOE_CODEC_H__
