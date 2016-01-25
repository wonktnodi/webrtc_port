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

 $Id: nf_est.h,v 1.2.2.2 2004/02/17 16:12:58 kk Exp $

****************************************************************************/
/*!
  \file
  \brief  Noise floor estimation structs and prototypes $Revision: 1.2.2.2 $
  \author Kristofer Kjoerling & Per Ekstrand
*/

#ifndef __NF_EST_H
#define __NF_EST_H

#include "sbr_main.h"
#include "sbr_def.h"
#include "fram_gen.h"


#define NF_SMOOTHING_LENGTH 4


typedef struct
{
  float prevNoiseLevels[NF_SMOOTHING_LENGTH][MAX_NUM_NOISE_VALUES];
  int freqBandTableQmf[MAX_NUM_NOISE_VALUES + 1];
  float ana_max_level;
  float weightFac;
  int noNoiseBands;
  int noiseBands;
  float noiseFloorOffset[MAX_NUM_NOISE_VALUES];
  const float* smoothFilter;
  INVF_MODE diffThres;
}
SBR_NOISE_FLOOR_ESTIMATE;

typedef SBR_NOISE_FLOOR_ESTIMATE *HANDLE_SBR_NOISE_FLOOR_ESTIMATE;

/*************/
/* Functions */
/*************/

void
sbrNoiseFloorEstimateQmf (HANDLE_SBR_NOISE_FLOOR_ESTIMATE h_sbrNoiseFloorEstimate,
                          const SBR_FRAME_INFO *frame_info,
                          float *noiseLevels,
                          float** quotaMatrixOrig,
                          char* indexVector,
                          int missingHarmonicsFlag,
                          int startIndex,
                          int numberOfEstiamtesPerFrame,
                          int totalNumberOfEstimates,
                          int transientFlag,
                          INVF_MODE* pInvFiltLevels
                          );


int
CreateSbrNoiseFloorEstimate (HANDLE_SBR_NOISE_FLOOR_ESTIMATE  h_sbrNoiseFloorEstimate,
                             int ana_max_level,
                             const unsigned char *freqBandTable,
                             int nSfb,
                             int noiseBands,
                             int noiseFloorOffset,
                             unsigned int useSpeechConfig
                             );



int
resetSbrNoiseFloorEstimate (HANDLE_SBR_NOISE_FLOOR_ESTIMATE h_sbrNoiseFloorEstimate,
                            const unsigned char *freqBandTable,
                            int nSfb);




void deleteSbrNoiseFloorEstimate (HANDLE_SBR_NOISE_FLOOR_ESTIMATE h_sbrNoiseFloorEstimate);

#endif
