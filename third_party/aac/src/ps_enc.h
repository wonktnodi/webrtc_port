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

 $Id: ps_enc.h,v 1.3.2.17 2004/04/01 13:14:17 ehr Exp $

*******************************************************************************/
/*!
  \file
  \brief  parametric stereo encoding $Revision: 1.3.2.17 $
  \author Jonas Engdegard
*/
#ifndef __PS_ENC_H
#define __PS_ENC_H

#include "ps_bitenc.h"
#include "hybrid_enc.h"
#include "psy_const.h"
#include <string>

/*############################################################################*/
/* Constant definitions                                                       */
/*############################################################################*/

#define NO_BINS                         ( 20 )
#define NO_LOW_RES_BINS                 ( NO_IID_BINS / 2 )

#define NO_IID_BINS                     ( NO_BINS )
#define NO_ICC_BINS                     ( NO_BINS )
#define NO_IPD_BINS                     ( 11 )
#define NO_IPD_BINS_EST                 ( NO_BINS )

#define NO_LOW_RES_IID_BINS             ( NO_LOW_RES_BINS )
#define NO_LOW_RES_ICC_BINS             ( NO_LOW_RES_BINS )

#define NO_IPD_GROUPS                   ( NO_IPD_BINS_EST + 6 + 2 )
#define NEGATE_IPD_MASK                 ( 0x00001000 )

#define ENERGY_WINDOW_SLOPES            ( 2 )

#define NO_SUBSAMPLES                   ( 32 )
#define NO_QMF_BANDS                    ( 64 )
#define SYSTEMLOOKAHEAD                 ( 1 )

/*############################################################################*/
/* Type definitions                                                           */
/*############################################################################*/

struct PS_ENC{

  int   bEnableHeader;

  int   bHiFreqResIidIcc;
  int   iidIccBins;

  unsigned int bPrevZeroIid;
  unsigned int bPrevZeroIcc;

  int psMode;

  struct BIT_BUF psBitBuf;

  int hdrBitsPrevFrame;

  float **aaaIIDDataBuffer;
  float **aaaICCDataBuffer;

  int   aLastIidIndex[NO_IID_BINS];
  int   aLastIccIndex[NO_ICC_BINS];

  float *mHybridRealLeft[NO_SUBSAMPLES];
  float *mHybridImagLeft[NO_SUBSAMPLES];
  float *mHybridRealRight[NO_SUBSAMPLES];
  float *mHybridImagRight[NO_SUBSAMPLES];

  HYBRID hybridLeft;
  HYBRID hybridRight;
  HANDLE_HYBRID hHybridLeft;
  HANDLE_HYBRID hHybridRight;

  float   powerLeft    [NO_BINS];
  float   powerRight   [NO_BINS];
  float   powerCorrReal[NO_BINS];
  float   powerCorrImag[NO_BINS];

  float **tempQmfLeftReal;
  float **tempQmfLeftImag;
  float **histQmfLeftReal;
  float **histQmfLeftImag;
  float **histQmfRightReal;
  float **histQmfRightImag;


  float mdctDelayBuffer_2[MAX_CHANNELS*BLOCK_SWITCHING_OFFSET];

  float PsBuf3[MAX_CHANNELS*FRAME_LEN_LONG*sizeof(short)/sizeof(float)];
  float PsBuf4[1024*4];
  
  PS_ENC(){
	  memset(aLastIidIndex, 0, sizeof(aLastIidIndex));
      memset(aLastIccIndex, 0, sizeof(aLastIccIndex));
	  memset(powerLeft, 0, sizeof(powerLeft));
	  memset(powerRight, 0, sizeof(powerRight));
	  memset(powerCorrReal, 0, sizeof(powerCorrReal));
	  memset(powerCorrImag,0, sizeof(powerCorrImag));
	  memset(mdctDelayBuffer_2, 0, sizeof(mdctDelayBuffer_2));
	  memset(PsBuf3, 0, sizeof(PsBuf3));
	  memset(PsBuf4, 0, sizeof(PsBuf4));
  }
};

typedef struct PS_ENC *HANDLE_PS_ENC;


/*############################################################################*/
/* Extern function prototypes                                                 */
/*############################################################################*/
int GetPsMode(int bitRate);

int
CreatePsEnc(HANDLE_PS_ENC h_ps_enc,
            int psMode,
			float* sbr_envYBuffer);

void
DeletePsEnc(HANDLE_PS_ENC *h_ps_e);


void
EncodePsFrame(HANDLE_PS_ENC h_ps_e,
              float **iBufferLeft,
              float **rBufferLeft,
              float **iBufferRight,
              float **rBufferRight);

#ifdef _cplusplus
};
#endif

#endif
