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

 $Id: env_calc.h,v 1.10.6.2 2004/02/17 15:36:16 jr Exp $

*******************************************************************************/
/*
  \file
  \brief  Envelope calculation prototypes $Revision: 1.10.6.2 $
  \author Per Ekstrand
*/
#ifndef __ENV_CALC_H
#define __ENV_CALC_H

#include "env_extr.h"  /* for HANDLE_SBR_HEADER_DATA_DEC */
#include "sbrdecsettings.h"

typedef struct
{
  float *filtBuffer;
  float *filtBufferNoise;

  int startUp;
  int phaseIndex;
  int prevTranEnv;

  int harmFlagsPrev[(MAX_FREQ_COEFFS+15)/16];

  char harmIndex;

}
SBR_CALCULATE_ENVELOPE;

typedef SBR_CALCULATE_ENVELOPE *HANDLE_SBR_CALCULATE_ENVELOPE;



void
calculateSbrEnvelope (HANDLE_SBR_CALCULATE_ENVELOPE h_sbr_cal_env,
                      HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
                      HANDLE_SBR_FRAME_DATA hFrameData,
                      float **analysBufferReal_m,
#ifndef LP_SBR_ONLY
                      float **analysBufferImag_m,
#endif
                      float *degreeAlias,
                      int bUseLP
                      );

int
createSbrEnvelopeCalc (HANDLE_SBR_CALCULATE_ENVELOPE hSbrCalculateEnvelope,
                       HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
                       int chan);

void
resetSbrEnvelopeCalc (HANDLE_SBR_CALCULATE_ENVELOPE hCalEnv);

int
ResetLimiterBands ( unsigned char *limiterBandTable,
                    unsigned char *noLimiterBands,
                    unsigned char *freqBandTable,
                    int noFreqBands,
                    const PATCH_PARAM *patchParam,
                    int noPatches,
                    int limiterBands);


#endif
