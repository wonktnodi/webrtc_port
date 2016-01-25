/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/audio_coding/codecs/amrwb/interface/amrwb_interface.h"

#include <stdlib.h>
#include <string.h>

#include "enc_if.h"
#include "dec_if.h"

#define AMRWB_DEBUG_MODE  1

#ifdef AMRWB_DEBUG_MODE
#define AMRWB_DEBUG_RECORD_ENC  0
#define AMRWB_DEBUG_RECORD_DEC  0
#include <stdio.h>
#include <time.h>
#define AMRWB_MAGIC_NUMBER "#!AMR-WB\n"
#endif  // AMRWB_DEBUG_MODE


struct AMRWB_encinst_t_ {
  void *encoder;
  Word16 dtx_mode;
#if AMRWB_DEBUG_RECORD_ENC
  FILE *enc_out_file;
#endif  // AMRWB_DEBUG_MODE
};


int16_t WebRtcAmrWb_CreateEnc(AMRWB_encinst_t_** inst) {
  AMRWB_encinst_t_ *state = 0;
  state = (AMRWB_encinst_t_*) calloc(1, sizeof(AMRWB_encinst_t_));

  if (state) {
    state->encoder = E_IF_init();
    state->dtx_mode = 1;
    if (state->encoder) {
      *inst = state;
      return 0;
    }

    free(state);
  }

  return -1;
}

int16_t WebRtcAmrWb_FreeEnc(AMRWB_encinst_t_* inst) {
#if AMRWB_DEBUG_RECORD_ENC
  if (inst->enc_out_file) {
    fclose(inst->enc_out_file);
    inst->enc_out_file = 0;
  }
#endif  // AMRWB_DEBUG_RECORD_ENC
  E_IF_exit(inst->encoder);
  free(inst);
  return 0;
}

int16_t WebRtcAmrWb_Encode(AMRWB_encinst_t_* inst, int16_t* audio_in,
                           int16_t samples, int16_t* encoded, int16_t mode) {
  Word16* audio = (Word16*) audio_in;
  UWord8 *coded = (UWord8 *)encoded;

  int res;

  if (samples > L_FRAME16k) {
    return -1;
  }

  res = E_IF_encode(inst->encoder, mode, audio, coded,
                    inst->dtx_mode);

  if (res > 0) {
#if AMRWB_DEBUG_RECORD_ENC
    fwrite(coded, 1, res, inst->enc_out_file);
#endif  // AMRWB_DEBUG_RECORD_ENC
    return res;
  }

  return -1;
}

int16_t WebRtcAmrWb_EncoderInit(AMRWB_encinst_t_* inst, int16_t dtx_mode) {  
#if AMRWB_DEBUG_RECORD_ENC
  char file_path[512] = {0};

  sprintf(file_path, "./%08x_%lu.awb", inst, time(0));
  inst->enc_out_file = fopen(file_path, "wb");
  if (inst->enc_out_file)
    fwrite(AMRWB_MAGIC_NUMBER, sizeof(char), strlen(AMRWB_MAGIC_NUMBER), inst->enc_out_file);
#endif  // AMRWB_DEBUG_RECORD_ENC
  
  inst->dtx_mode = dtx_mode;
  return 0;
}

int16_t WebRtcAmrWb_EncodeBitmode(AMRWB_encinst_t_* enc_inst, int format) {
  return 0;
}

struct AMRWB_decinst_t_ {
  void *decoder;
#if AMRWB_DEBUG_RECORD_DEC
  FILE *dec_out_file;
#endif  // AMRWB_DEBUG_RECORD_DEC
};

int16_t WebRtcAmrWb_CreateDec(AMRWB_decinst_t_** inst) {
  AMRWB_decinst_t_ *state = 0;
  state = (AMRWB_decinst_t_*) calloc(1, sizeof(AMRWB_decinst_t_));

  if (state) {
    state->decoder = D_IF_init();

    if (state->decoder) {
#if AMRWB_DEBUG_RECORD_DEC
      state->dec_out_file = 0;
#endif  // AMRWB_DEBUG_RECORD_DEC
      *inst = state;
      return 0;
    }

    free(state);
  }

  return -1;
}

int16_t WebRtcAmrWb_FreeDec(AMRWB_decinst_t_* inst) {
  D_IF_exit(inst->decoder);
#if AMRWB_DEBUG_RECORD_DEC
  if (inst->dec_out_file) {
    fclose(inst->dec_out_file);
    inst->dec_out_file = 0;
  }
#endif  // AMRWB_DEBUG_RECORD_DEC
  free(inst);
  return 0;
}

typedef struct {
  Word16 reset_flag_old;     /* previous was homing frame  */
  Word16 prev_ft;            /* previous frame type        */
  Word16 prev_mode;          /* previous mode              */
  void *decoder_state;       /* Points decoder state       */
} WB_dec_if_state;

int16_t WebRtcAmrWb_Decode(AMRWB_decinst_t_* inst, int16_t* encoded,
                           int16_t encoded_bytes, int16_t* decoded, int16_t* audio_type) {
#if AMRWB_DEBUG_RECORD_DEC
  fwrite((UWord8*)encoded, 1, encoded_bytes, inst->dec_out_file);
#endif  // AMRWB_DEBUG_RECORD_DEC  
  D_IF_decode(inst->decoder, (UWord8*)encoded, decoded, _good_frame);
  return L_FRAME16k;
}

int16_t WebRtcAmrWb_DecodePlc(AMRWB_decinst_t_* dec_inst) {
  return -1;
}

int16_t WebRtcAmrWb_DecoderInit(AMRWB_decinst_t_* inst) {
#if AMRWB_DEBUG_RECORD_DEC
  char file_path[512] = {0};

  sprintf(file_path, "./%08x_%lu.awb", inst, time(0));
  inst->dec_out_file = fopen(file_path, "wb");
  if (inst->dec_out_file)
    fwrite(AMRWB_MAGIC_NUMBER, sizeof(char), strlen(AMRWB_MAGIC_NUMBER), inst->dec_out_file);
#endif  // AMRWB_DEBUG_RECORD_DEC

  return 0;
}

int16_t WebRtcAmrWb_DecodeBitmode(AMRWB_decinst_t_* dec_inst, int format) {  
  return 0;
}