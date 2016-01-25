/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_CODING_CODECS_AMRWB_INTERFACE_OPUS_INTERFACE_H_
#define WEBRTC_MODULES_AUDIO_CODING_CODECS_AMRWB_INTERFACE_OPUS_INTERFACE_H_

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

// Opaque wrapper types for the codec state.
typedef struct AMRWB_encinst_t_ AMRWB_encinst_t_;
typedef struct AMRWB_decinst_t_ AMRWB_decinst_t_;

int16_t WebRtcAmrWb_CreateEnc(AMRWB_encinst_t_** enc_inst);
int16_t WebRtcAmrWb_FreeEnc(AMRWB_encinst_t_* enc_inst);

/****************************************************************************
 * WebRtcAmrWb_Encode(...)
 *
 * This function encodes audio as a series of AMR-WB frames and inserts
 * it into a packet. Input buffer can be any length.
 *
 * Input:
 *      - enc_inst                  : Encoder context
 *      - input              : Input speech data buffer
 *      - len               : Samples in audio_in
 *      - output : Output buffer size
 *
 * Output:
 *      - encoded               : Output compressed data buffer
 *
 * Return value                 : >0 - Length (in bytes) of coded data
 *                                -1 - Error
 */
int16_t WebRtcAmrWb_Encode(AMRWB_encinst_t_* enc_inst, int16_t* input,
  int16_t samples, int16_t* output, int16_t mode);

/****************************************************************************
 * WebRtcAmrWb_EncoderInit(...)
 *
 */
int16_t WebRtcAmrWb_EncoderInit(AMRWB_encinst_t_* enc_inst,
                                int16_t dtx_mode);

/****************************************************************************
 * WebRtcAmrWb_EncodeBitmode(...)
 *
 */
int16_t WebRtcAmrWb_EncodeBitmode(AMRWB_encinst_t_* enc_inst,
                                   int format);


int16_t WebRtcAmrWb_CreateDec(AMRWB_decinst_t_** dec_inst);
int16_t WebRtcAmrWb_FreeDec(AMRWB_decinst_t_* dec_inst);

/****************************************************************************
 * WebRtcAmrWb_Decode(...)
 *
 */
int16_t WebRtcAmrWb_Decode(AMRWB_decinst_t_* inst, int16_t* encoded,
                           int16_t encoded_bytes, int16_t* decoded, int16_t* audio_type);

/****************************************************************************
 * WebRtcAmrWb_DecodePlc(...)
 *
 */
int16_t WebRtcAmrWb_DecodePlc(AMRWB_decinst_t_* dec_inst);

/****************************************************************************
 * WebRtcAmrWb_DecoderInit(...)
 *
 */
int16_t WebRtcAmrWb_DecoderInit(AMRWB_decinst_t_* dec_inst);

/****************************************************************************
 * WebRtcAmrWb_DecodeBitmode(...)
 *
 */
int16_t WebRtcAmrWb_DecodeBitmode(AMRWB_decinst_t_* dec_inst,
                                  int format);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // WEBRTC_MODULES_AUDIO_CODING_CODECS_AMRWB_INTERFACE_OPUS_INTERFACE_H_
