/****************************************************************************

                      (C) copyright Coding Technologies (2004)
                               All Rights Reserved

 This software module was developed by Coding Technologies (CT). This is
 company confidential information and the property of CT, and can not be
 reproduced or disclosed in any form without written authorization of CT.

 Those intending to use this software module for other purposes are advised
 that this infringe existing or pending patents. CT has no liability for
 use of this software module or derivatives thereof in any implementation.
 Copyright is not released for any means.  CT retains full right to use the
 code for its own purpose, assign or sell the code to a third party and to
 inhibit any user or third party from using the code. This copyright notice
 must be included in all copies or derivative works.

 $Id: fram_gen.h,v 1.3.2.3 2004/02/17 10:21:21 jr Exp $

*******************************************************************************/
/*!
  \file
  \brief  Framing generator prototypes and structs $Revision: 1.3.2.3 $
  \author Fredrik Henn (MATLAB template code), Kristofer Kjoerling (C-port)
*/
#ifndef _FRAM_GEN_H
#define _FRAM_GEN_H

#include "sbr_def.h"
#include "sbr_main.h"

#define MAX_ENVELOPES_VARVAR MAX_ENVELOPES
#define MAX_ENVELOPES_FIXVAR_VARFIX 4
#define MAX_NUM_REL 3

/* SBR frame class definitions */
typedef enum {
  FIXFIX = 0,
  FIXVAR,
  VARFIX,
  VARVAR
}FRAME_CLASS;


#define DC      4711
#define EMPTY   (-99)



typedef struct
{


  FRAME_CLASS frameClass;
  int bs_num_env;
  int bs_abs_bord;
  int n;
  int p;
  int bs_rel_bord[MAX_NUM_REL];
  FREQ_RES v_f[MAX_ENVELOPES_FIXVAR_VARFIX];

  int bs_abs_bord_0;
  int bs_abs_bord_1;
  int bs_num_rel_0;
  int bs_num_rel_1;
  int bs_rel_bord_0[MAX_NUM_REL];
  int bs_rel_bord_1[MAX_NUM_REL];
  FREQ_RES v_fLR[MAX_ENVELOPES_VARVAR];

}
SBR_GRID;
typedef SBR_GRID *HANDLE_SBR_GRID;


/*!
  \struct SBR_FRAME_INFO
*/
typedef struct
{
  int nEnvelopes;
  int borders[MAX_ENVELOPES+1];
  FREQ_RES freqRes[MAX_ENVELOPES];
  int shortEnv;
  int nNoiseEnvelopes;
  int bordersNoise[MAX_NOISE_ENVELOPES+1];
}
SBR_FRAME_INFO;

typedef SBR_FRAME_INFO *HANDLE_SBR_FRAME_INFO;

/*!
  \struct SBR_ENVELOPE_FRAME
*/
typedef struct
{
  int staticFraming;
  int numEnvStatic;
  FREQ_RES freq_res_fixfix;

  int *v_tuningSegm;
  int *v_tuningFreq;
  int dmin;
  int dmax;
  int allowSpread;

  FRAME_CLASS frameClassOld;
  int spreadFlag;

  int v_bord[2 * MAX_ENVELOPES_VARVAR + 1];
  int length_v_bord;
  FREQ_RES v_freq[2 * MAX_ENVELOPES_VARVAR + 1];
  int length_v_freq;

  int v_bordFollow[MAX_ENVELOPES_VARVAR];
  int length_v_bordFollow;
  int i_tranFollow;
  int i_fillFollow;
  FREQ_RES v_freqFollow[MAX_ENVELOPES_VARVAR];
  int length_v_freqFollow;

  SBR_GRID         SbrGrid;
  SBR_FRAME_INFO   SbrFrameInfo;
}
SBR_ENVELOPE_FRAME;
typedef SBR_ENVELOPE_FRAME *HANDLE_SBR_ENVELOPE_FRAME;



void
CreateFrameInfoGenerator (HANDLE_SBR_ENVELOPE_FRAME  hSbrEnvFrame,
                          int allowSpread,
                          int numEnvStatic,
                          int staticFraming,

                          FREQ_RES freq_res_fixfix);

void
deleteFrameInfoGenerator (HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame);

HANDLE_SBR_FRAME_INFO
frameInfoGenerator (HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame,
                    int *v_pre_transient_info,
                    int *v_transient_info,
                    int *v_tuning);

#endif
