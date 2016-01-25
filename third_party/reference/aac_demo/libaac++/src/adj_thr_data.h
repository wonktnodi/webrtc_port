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
 
 $Id: adj_thr_data.h,v 1.2.2.2 2004/02/16 20:02:18 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  threshold calculations $Revision: 1.2.2.2 $
  \author M. Schug / A. Groeschel
*/
#ifndef __ADJ_THR_DATA_H
#define __ADJ_THR_DATA_H

#include "psy_const.h"

typedef struct {
   float clipSaveLow, clipSaveHigh;
   float minBitSave, maxBitSave;
   float clipSpendLow, clipSpendHigh;
   float minBitSpend, maxBitSpend;
} BRES_PARAM;

typedef struct {
   unsigned char modifyMinSnr;
   int startSfbL, startSfbS;
} AH_PARAM;

typedef struct {
  float maxRed;
  float startRatio, maxRatio;
  float redRatioFac, redOffs;
} MINSNR_ADAPT_PARAM;

typedef struct {
  float peMin, peMax;
  float peOffset;
  AH_PARAM ahParam;
  MINSNR_ADAPT_PARAM minSnrAdaptParam;
  float peLast;
  int dynBitsLast;
  float peCorrectionFactor;
} ATS_ELEMENT;

typedef struct {
  BRES_PARAM   bresParamLong, bresParamShort;
  ATS_ELEMENT  adjThrStateElem;
} ADJ_THR_STATE;

#endif
