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

 $Id: sbr_dec.h,v 1.4.6.9 2004/04/01 07:31:14 ehr Exp $

*******************************************************************************/
/*
  \file
  \brief  Sbr decoder $Revision: 1.4.6.9 $
  \author Per Ekstrand
*/
#ifndef __SBR_DEC_H
#define __SBR_DEC_H

#include "sbr_const.h"
#include "lpp_tran.h"
#include "qmf_dec.h"
#include "env_calc.h"


typedef struct
{
  float * ptrsbr_OverlapBuffer;

  SBR_QMF_FILTER_BANK_DEC     CodecQmfBank;
  SBR_QMF_FILTER_BANK_DEC     SynthesisQmfBank;
  SBR_CALCULATE_ENVELOPE  SbrCalculateEnvelope;
  SBR_LPP_TRANS           LppTrans;

  unsigned char qmfLpChannel;
  unsigned char bApplyQmfLp;
  float        *workBuffer2;
}
SBR_DEC;

typedef SBR_DEC *HANDLE_SBR_DEC;


typedef struct
{
  HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData;
  SBR_PREV_FRAME_DATA PrevFrameData[MAXNRSBRCHANNELS];
  SBR_DEC SbrDec;
}
SBR_CHANNEL;


typedef struct
{
  SBRBITSTREAM * Bitstream;
  SBRBITSTREAM sbr_PrevBitstream;
  int          FrameOk;
}
SBR_CONCEAL_DATA;

typedef SBR_CONCEAL_DATA *HANDLE_SBR_CONCEAL_DATA;

void sbr_dec (HANDLE_SBR_DEC hSbrDec,
              float *timeIn,
              float *timeOut,
              float *interimResult,
              HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
              HANDLE_SBR_FRAME_DATA hFrameData,
              HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData,
              int applyProcessing,
              struct PS_DEC *h_ps_d,
              SBR_QMF_FILTER_BANK_DEC *hSynthesisQmfBankRight
#ifdef NON_BE_BUGFIX
              , int nChannels
#endif
              );

int
createSbrDec (SBR_CHANNEL * hSbrChannel,
              HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
              int chan,
              int bApplyQmfLp,
              int sampleFreq);
int
createSbrQMF (SBR_CHANNEL * hSbrChannel,
              HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
              int chan,
              int bDownSample);

int
resetSbrQMF (HANDLE_SBR_DEC hSbrDec,
             HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
             int sbrChannel,
#ifdef NON_BE_BUGFIX
             int nChannels,
#endif
             HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData);

#endif
