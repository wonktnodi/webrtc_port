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

 $Id: ps_dec.h,v 1.5.4.13 2004/04/23 10:29:01 je Exp $

*******************************************************************************/
/*
  \file
  \brief  parametric stereo decoding structs and prototypes $Revision: 1.5.4.13 $
  \author Jonas Engdegard
*/
#ifndef __PS_DEC_H
#define __PS_DEC_H

#include "sbr_bitb.h"
#include "sbrdecoder.h"
#include "hybrid_dec.h"
#include "sbrdecsettings.h"
#include "env_extr.h"
#define NO_SUB_QMF_CHANNELS         12
#define NO_QMF_CHANNELS_IN_HYBRID   3
#define NO_QMF_CHANNELS             64
#define NO_ALLPASS_CHANNELS         23
#define NO_DELAY_CHANNELS           (NO_QMF_CHANNELS-NO_ALLPASS_CHANNELS)
#define DELAY_ALLPASS               2
#define SHORT_DELAY_START           12
#define SHORT_DELAY                 1
#define LONG_DELAY                  14
#define NO_QMF_ALLPASS_CHANNELS    (NO_ALLPASS_CHANNELS-NO_QMF_CHANNELS_IN_HYBRID)
#define NO_QMF_ICC_CHANNELS        (NO_QMF_ALLPASS_CHANNELS+NO_DELAY_CHANNELS)
#define HYBRIDGROUPS                8
#define DECAY_CUTOFF                3
#define DECAY_SLOPE                 0.05f
#define PEAK_DECAY_FACTOR           0.765928338364649f
#define NO_SERIAL_ALLPASS_LINKS     3
#define MAX_NO_PS_ENV               5
#define PI            3.1415926535897932
#define PSC_SQRT2                       ( 1.41421356237309504880 )
#define PSC_PIF                         ( ( float )PI )
#define PSC_2PIF                        ( ( float )( 2 * PI ) )
#define PSC_PI2F                        ( ( float )( PI / 2 ) )
#define PSC_SQRT2F                      ( ( float )PSC_SQRT2 )
#define NEGATE_IPD_MASK                 ( 0x00001000 )
#define NRG_INT_COEFF                    0.25f
#define NO_BINS                         ( 20 )
#define NO_HI_RES_BINS                  ( 34 )
#define NO_LOW_RES_BINS                 ( NO_IID_BINS / 2 )
#define NO_IID_BINS                     ( NO_BINS )
#define NO_ICC_BINS                     ( NO_BINS )
#define NO_LOW_RES_IID_BINS             ( NO_LOW_RES_BINS )
#define NO_LOW_RES_ICC_BINS             ( NO_LOW_RES_BINS )
#define SUBQMF_GROUPS                   ( 10 )
#define QMF_GROUPS                      ( 12 )
#define NO_IID_GROUPS                   ( SUBQMF_GROUPS + QMF_GROUPS )
#define NO_IID_STEPS                    ( 7 )
#define NO_IID_STEPS_FINE               ( 15 )
#define NO_ICC_STEPS                    ( 8 )
#define NO_IID_LEVELS                   ( 2 * NO_IID_STEPS + 1 )
#define NO_IID_LEVELS_FINE              ( 2 * NO_IID_STEPS_FINE + 1 )
#define NO_ICC_LEVELS                   ( NO_ICC_STEPS )

/****************************************************************
  Type definitions
 ****************************************************************/
struct PS_DEC {
  int bForceMono;
  unsigned int noSubSamples;
  float invNoSubSamples;
  int usb;
  int lastUsb;

  int bPsDataAvail;

  int bEnableIid;
  int bEnableIcc;

#ifdef NON_BE_BUGFIX
  int bEnableExt;
  int bFineIidQ;
  int aIidPrevFrameIndex[NO_HI_RES_BINS];
  int aIccPrevFrameIndex[NO_HI_RES_BINS];
  int aaIidIndex[MAX_NO_PS_ENV+1][NO_HI_RES_BINS];
  int aaIccIndex[MAX_NO_PS_ENV+1][NO_HI_RES_BINS];
#else
  int aIidPrevFrameIndex[NO_BINS];
  int aIccPrevFrameIndex[NO_BINS];
  int aaIidIndex[MAX_NO_PS_ENV+1][NO_BINS];
  int aaIccIndex[MAX_NO_PS_ENV+1][NO_BINS];
#endif

  int freqResIid;
  int freqResIcc;

  int bFrameClass;
  int noEnv;
  int aEnvStartStop[MAX_NO_PS_ENV+1];

  int abIidDtFlag[MAX_NO_PS_ENV];
  int abIccDtFlag[MAX_NO_PS_ENV];

  int   delayBufIndex;
  float **aaRealDelayBufferQmf;
  float **aaImagDelayBufferQmf;

  unsigned int aDelayRBufIndexSer[NO_SERIAL_ALLPASS_LINKS];
  unsigned int aNoSampleDelayRSer[NO_SERIAL_ALLPASS_LINKS];

  float **aaaRealDelayRBufferSerQmf[NO_SERIAL_ALLPASS_LINKS];
  float **aaaImagDelayRBufferSerQmf[NO_SERIAL_ALLPASS_LINKS];

  float **aaaRealDelayRBufferSerSubQmf[NO_SERIAL_ALLPASS_LINKS];
  float **aaaImagDelayRBufferSerSubQmf[NO_SERIAL_ALLPASS_LINKS];

  float **aaRealDelayBufferSubQmf;
  float **aaImagDelayBufferSubQmf;

  float *aPeakDecayFast;
  float *aPrevNrg;
  float *aPrevPeakDiff;

  float *mHybridRealLeft;
  float *mHybridImagLeft;
  float *mHybridRealRight;
  float *mHybridImagRight;

  HANDLE_HYBRID hHybrid;

  int aDelayBufIndex[NO_DELAY_CHANNELS];
  int aNoSampleDelay[NO_DELAY_CHANNELS];

  float h11Prev[NO_IID_GROUPS];
  float h12Prev[NO_IID_GROUPS];
  float h21Prev[NO_IID_GROUPS];
  float h22Prev[NO_IID_GROUPS];

  float H11[NO_IID_GROUPS];
  float H12[NO_IID_GROUPS];
  float H21[NO_IID_GROUPS];
  float H22[NO_IID_GROUPS];

  float deltaH11[NO_IID_GROUPS];
  float deltaH12[NO_IID_GROUPS];
  float deltaH21[NO_IID_GROUPS];
  float deltaH22[NO_IID_GROUPS];

  PS_DEC(){
	  memset(aEnvStartStop, 0 , sizeof(aEnvStartStop));
	  memset(abIidDtFlag, 0, sizeof(abIidDtFlag));
	  memset(abIccDtFlag, 0, sizeof(abIccDtFlag));
	  memset(aDelayRBufIndexSer, 0, sizeof(aDelayRBufIndexSer));
	  memset(aNoSampleDelayRSer, 0, sizeof(aNoSampleDelayRSer));
	  memset(aDelayBufIndex, 0, sizeof(aDelayBufIndex));
	  memset(aNoSampleDelay, 0, sizeof(aNoSampleDelay));
	  memset(h11Prev, 0, sizeof(h11Prev));
      memset(h12Prev, 0, sizeof(h12Prev));
	  memset(h21Prev, 0, sizeof(h21Prev));
	  memset(h22Prev, 0, sizeof(h22Prev));

	  memset(H11, 0, sizeof(H11));
	  memset(H12, 0, sizeof(H12));
	  memset(H21, 0, sizeof(H21));
	  memset(H22, 0, sizeof(H22));
	  memset(deltaH11, 0, sizeof(deltaH11));
      memset(deltaH12, 0, sizeof(deltaH12));
	  memset(deltaH21, 0, sizeof(deltaH21));
	  memset(deltaH22, 0, sizeof(deltaH22));

  }
};

typedef struct PS_DEC *HANDLE_PS_DEC;

/****************************************************************
  Functions
 ****************************************************************/
int
CreatePsDec(HANDLE_PS_DEC h_ps_dec,
			HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
            HANDLE_SBR_LPP_TRANS hLppTrans,
			unsigned int noSubSamples,
			float* overlapBuf);

void
InitRotationEnvelope(HANDLE_PS_DEC h_ps_dec,
                     int env,
                     int usb);
void
ApplyPsSlot(HANDLE_PS_DEC h_ps_dec,
            float **rBufferLeft,
            float **iBufferLeft,
            float *rBufferRight,
            float *iBufferRight);
#endif
