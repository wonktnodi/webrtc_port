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

 $Id: code_env.h,v 1.2.2.1 2004/02/17 10:21:21 jr Exp $

*******************************************************************************/
/*!
  \file
  \brief  DPCM Envelope coding $Revision: 1.2.2.1 $
  \author Kristofer Kjoerling & Per Ekstrand
*/

#ifndef __CODE_ENV_H
#define __CODE_ENV_H

#include "sbr_main.h"
#include "sbr_def.h"
#include "fram_gen.h"

typedef struct
{
  int offset;
  int upDate;
  int nSfb[2];
  int sfb_nrg_prev[MAX_FREQ_COEFFS];
  int deltaTAcrossFrames;
  float dF_edge_1stEnv;
  float dF_edge_incr;
  int dF_edge_incr_fac;

  int codeBookScfLavTime;
  int codeBookScfLavFreq;

  int codeBookScfLavLevelTime;
  int codeBookScfLavLevelFreq;
  int codeBookScfLavBalanceTime;
  int codeBookScfLavBalanceFreq;

  int start_bits;
  int start_bits_balance;


  const unsigned char *hufftableTimeL;
  const unsigned char *hufftableFreqL;

  const unsigned char *hufftableLevelTimeL;
  const unsigned char *hufftableBalanceTimeL;
  const unsigned char *hufftableLevelFreqL;
  const unsigned char *hufftableBalanceFreqL;
}
SBR_CODE_ENVELOPE;
typedef SBR_CODE_ENVELOPE *HANDLE_SBR_CODE_ENVELOPE;



void
codeEnvelope (int *sfb_nrg,
              const FREQ_RES *freq_res,
              SBR_CODE_ENVELOPE * h_sbrCodeEnvelope,
              int *directionVec, int coupling, int nEnvelopes, int channel,
              int headerActive);

int
CreateSbrCodeEnvelope (HANDLE_SBR_CODE_ENVELOPE h_sbrCodeEnvelope,
                       int *nSfb,
                       int deltaTAcrossFrames,
                       float dF_edge_1stEnv,
                       float dF_edge_incr);

void deleteSbrCodeEnvelope (HANDLE_SBR_CODE_ENVELOPE h_sbrCodeEnvelope);

struct SBR_ENV_DATA;

int
InitSbrHuffmanTables (struct SBR_ENV_DATA*      sbrEnvData,
                      HANDLE_SBR_CODE_ENVELOPE  henv,
                      HANDLE_SBR_CODE_ENVELOPE  hnoise,
                      AMP_RES                   amp_res);

#endif
