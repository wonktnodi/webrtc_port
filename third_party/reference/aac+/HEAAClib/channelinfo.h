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

 $Id: channelinfo.h,v 1.8.6.5 2004/02/16 15:52:53 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  individual channel stream info  $Revision: 1.8.6.5 $
  \author zerok
*/

#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include "aacdecoder.h"
#include "overlapadd.h"
#include "pulsedata.h"
#include "streaminfo.h"

#include "FFR_bitbuffer.h"

#include "channelinfo2.h"

enum
{
  OnlyLongSequence = 0,
  LongStartSequence,
  EightShortSequence,
  LongStopSequence,

  ProfileMain = 0,
  ProfileLowComplexity,
  ProfileSSR,
  ProfileReserved
};

typedef struct
{
  int SamplingFrequency;
  char NumberOfScaleFactorBands_Long;
  const short *ScaleFactorBands_Long;
  char NumberOfScaleFactorBands_Short;
  const short *ScaleFactorBands_Short;
} SamplingRateInfo;

typedef struct
{
  char  WindowShape;
  char  WindowSequence;
  float SpectralCoefficient[FRAME_SIZE];

  int   iRandomPhase;
  int   prevFrameOk[2];
  int   cntConcealFrame;
  int   ConcealState;
  int   nFadeInFrames;
  int   nValidFrames;
  int   nFadeOutFrames;

} CConcealmentInfo;

typedef struct _tagCAacDecoderStaticChannelInfo{
  const float *pLongWindow[2];
  const float *pShortWindow[2];

  COverlapAddData OverlapAddData;

  CConcealmentInfo *hConcealmentInfo;

  CConcealmentInfo ConcealmentInfo[2];
} CAacDecoderStaticChannelInfo;

int IcsRead(HANDLE_BIT_BUF bs,CIcsInfo *pIcsInfo);
void IcsReset(CIcsInfo *pIcsInfo, CStreamInfo *pStreamInfo);

unsigned char IsValid(CIcsInfo *pIcsInfo);
unsigned char IsLongBlock(CIcsInfo *pIcsInfo);
unsigned char IsShortBlock(CIcsInfo *pIcsInfo);
unsigned char IsMainProfile(CIcsInfo *pIcsInfo);
int  GetProfile(CIcsInfo *pIcsInfo);
char GetWindowShape(CIcsInfo *pIcsInfo);
char GetWindowSequence(CIcsInfo *pIcsInfo);
int  GetWindowsPerFrame(CIcsInfo *pIcsInfo);
char GetWindowGroups(CIcsInfo *pIcsInfo);
char GetWindowGroupLength(CIcsInfo *pIcsInfo, int index);
char *GetWindowGroupLengthTable(CIcsInfo *pIcsInfo);
char GetScaleFactorBandsTransmitted(CIcsInfo *pIcsInfo);
char GetScaleFactorBandsTotal(CIcsInfo *pIcsInfo);
const short *GetScaleFactorBandOffsets(CIcsInfo *pIcsInfo);
int SamplingRateFromIndex(int index);
int GetSamplingFrequency(CIcsInfo *pIcsInfo);
char GetMaximumTnsBands(CIcsInfo *pIcsInfo);
#endif /* #ifndef CHANNELINFO_H */
