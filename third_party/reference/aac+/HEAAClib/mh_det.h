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

 $Id: mh_det.h,v 1.2.2.5 2004/04/01 13:14:17 ehr Exp $

*******************************************************************************/
/*!
  \file
  \brief  missing harmonics detection header file $Revision: 1.2.2.5 $
  \author Kristofer Kjoerling
*/

#ifndef __MH_DETECT_H
#define __MH_DETECT_H


#include "sbr_main.h"
#include "fram_gen.h"



typedef struct
{
  float* guideVectorDiff;
  float* guideVectorOrig;
  unsigned char* guideVectorDetected;
}GUIDE_VECTORS;





typedef struct
{
  int qmfNoChannels;
  int nSfb;
  int sampleFreq;
  int previousTransientFlag;
  int previousTransientFrame;
  int previousTransientPos;

  int noVecPerFrame;
  int transientPosOffset;

  int move;
  int totNoEst;
  int noEstPerFrame;
  int timeSlots;

  unsigned char  sbr_guideScfb[MAX_CHANNELS*MAX_FREQ_COEFFS];
  unsigned char* guideScfb;

  char     sbr_prevEnvelopeCompensation[MAX_CHANNELS*MAX_FREQ_COEFFS];
  char *prevEnvelopeCompensation;

  unsigned char   sbr_detectionVectors[MAX_CHANNELS*NO_OF_ESTIMATES*MAX_FREQ_COEFFS];
  unsigned char* detectionVectors[NO_OF_ESTIMATES];
  float* tonalityDiff[NO_OF_ESTIMATES];
  float* sfmOrig[NO_OF_ESTIMATES];
  float* sfmSbr[NO_OF_ESTIMATES];

  unsigned char   sbr_guideVectorDetected[MAX_CHANNELS*NO_OF_ESTIMATES*MAX_FREQ_COEFFS];
  GUIDE_VECTORS guideVectors[NO_OF_ESTIMATES];

  float sbr_toncorrBuff[1024*4];
}
SBR_MISSING_HARMONICS_DETECTOR;

typedef SBR_MISSING_HARMONICS_DETECTOR *HANDLE_SBR_MISSING_HARMONICS_DETECTOR;



void
SbrMissingHarmonicsDetectorQmf(HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector,
                               float ** pQuotaBuffer,
                               char *indexVector,
                               const SBR_FRAME_INFO *pFrameInfo,
                               const int* pTranInfo,
                               int* pAddHarmonicsFlag,
                               unsigned char* pAddHarmonicsScaleFactorBands,
                               const unsigned char* freqBandTable,
                               int nSfb,
                               char * envelopeCompensation);


int
CreateSbrMissingHarmonicsDetector(int chan,
                                  HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector,
                                  int sampleFreq,
                                  unsigned char* freqBandTable,
                                  int nSfb,
                                  int qmfNoChannels,
                                  int totNoEst,
                                  int move,
                                  int noEstPerFrame);

#endif
