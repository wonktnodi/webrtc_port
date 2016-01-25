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
 
 $Id: psy_main.h,v 1.3.2.4 2004/05/28 11:36:18 snd Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Psychoaccoustic main module $Revision: 1.3.2.4 $
  \author M. Werner
*/
#ifndef _PSYMAIN_H
#define _PSYMAIN_H

#include "psy_configuration.h"
#include "qc_data.h"


/*
  psy kernel
*/
typedef struct  {
  PSY_CONFIGURATION_LONG  psyConfLong;
  PSY_CONFIGURATION_SHORT psyConfShort;

  float				 mdctDelayBuffer_2[MAX_CHANNELS*BLOCK_SWITCHING_OFFSET];
  PSY_DATA           psyData[MAX_CHANNELS];
  TNS_DATA           tnsData[MAX_CHANNELS];

  float  sbr_envIBuffer [MAX_CHANNELS * QMF_TIME_SLOTS * QMF_CHANNELS];
  float*             pScratchTns;

  float  sbr_envRBuffer[MAX_CHANNELS * QMF_TIME_SLOTS * QMF_CHANNELS];
}PSY_KERNEL;


int PsyNew( PSY_KERNEL  *hPsy, int nChan);
int PsyDelete( PSY_KERNEL  *hPsy);

int PsyOutNew( PSY_OUT *hPsyOut);
int PsyOutDelete( PSY_OUT *hPsyOut);

int psyMainInit( PSY_KERNEL *hPsy,
                 int sampleRate,
                 int bitRate,
                 int channels,
                 int tnsMask,
                 int bandwidth);


int psyMain(int                     nChannels, /*! total number of channels */              
            ELEMENT_INFO            *elemInfo,
            float                   *timeSignal, /*! interleaved time signal */ 
            PSY_DATA                 psyData[MAX_CHANNELS],
            TNS_DATA                 tnsData[MAX_CHANNELS],
            PSY_CONFIGURATION_LONG  *psyConfLong,
            PSY_CONFIGURATION_SHORT *psyConfShort,
            PSY_OUT_CHANNEL          psyOutChannel[MAX_CHANNELS],
            PSY_OUT_ELEMENT         *psyOutElement,
            float                   *pScratchTns);

#endif /* _PSYMAIN_H */

