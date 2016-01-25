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
 
 $Id: interface.h,v 1.6.2.4 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  Interface psychoaccoustic/quantizer $Revision: 1.6.2.4 $
  \author M. Werner
*/
#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "psy_const.h"
#include "psy_data.h"
#include "sbr_def.h"
#include <string>

enum
{
  MS_NONE = 0,
  MS_SOME = 1,
  MS_ALL  = 2
};

enum
{
  MS_ON = 1
};

struct TOOLSINFO {
  int msDigest; /* 0 = no MS; 1 = some MS, 2 = all MS */
  int msMask[MAX_GROUPED_SFB];
};


typedef struct  __tagPSY_OUT_CHANNEL{
  int   sfbCnt;
  int   sfbPerGroup;
  int   maxSfbPerGroup;
  int   windowSequence;
  int   windowShape;
  int   groupingMask;
  int   sfbOffsets[MAX_GROUPED_SFB+1];
  float *sfbEnergy;
  float *sfbSpreadedEnergy;
  float *sfbThreshold;
  float *mdctSpectrum;
  
  float sfbEnSumLR;
  float sfbEnSumMS;
  float pe;   /* pe sum for stereo preprocessing */
  
  float sfbMinSnr[MAX_GROUPED_SFB];
  float expSpec[MAX_CHANNELS * QMF_TIME_SLOTS * QMF_CHANNELS];
  TNS_INFO tnsInfo;

  short quantSpecTmp[4096];
  __tagPSY_OUT_CHANNEL()
  {
    memset(expSpec, 0, sizeof(expSpec));
  }
 }PSY_OUT_CHANNEL;

typedef struct {
  struct TOOLSINFO toolsInfo;
  float weightMsLrPeRatio;      /*! factor to weight PE dependent on ms vs. lr PE ratio */
} PSY_OUT_ELEMENT;

typedef struct {
  /* information shared by both channels  */
  PSY_OUT_ELEMENT  psyOutElement;
  /* information specific to each channel */
  PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS];
}PSY_OUT;

void BuildInterface(float                  *mdctSpectrum,
                    SFB_THRESHOLD          *sfbThreshold,
                    SFB_ENERGY             *sfbEnergy,
                    SFB_ENERGY             *sfbSpreadedEnergy,
                    const SFB_ENERGY_SUM    sfbEnergySumLR,
                    const SFB_ENERGY_SUM    sfbEnergySumMS,
                    const int               windowSequence,
                    const int               windowShape,
                    const int               sfbCnt,
                    const int              *sfbOffset,
                    const int               maxSfbPerGroup,
                    const float           *groupedSfbMinSnr,
                    const int               noOfGroups,
                    const int              *groupLen,
                    PSY_OUT_CHANNEL        *psyOutCh);

#endif /* _INTERFACE_H */
