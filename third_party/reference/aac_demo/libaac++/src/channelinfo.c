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

 $Id: channelinfo.c,v 1.1.2.6 2004/02/16 17:53:57 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  individual channel stream info  $Revision: 1.1.2.6 $
  \author zerok
*/

#include "aac_rom_dec.h"
#include "streaminfo.h"
#include "FFR_bitbuffer.h"

#include "flc.h" /* the 3GPP instrumenting tool */

unsigned char IsValid(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("IsValid");
  INDIRECT(1);
  FLC_sub_end();

  return pIcsInfo->Valid;
}

unsigned char IsLongBlock(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("IsLongBlock");
  INDIRECT(1); ADD(1); BRANCH(1);
  FLC_sub_end();

  return (pIcsInfo->WindowSequence != EightShortSequence);
}

unsigned char IsShortBlock(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("IsShortBlock");
  INDIRECT(1); ADD(1); BRANCH(1);
  FLC_sub_end();

  return (pIcsInfo->WindowSequence == EightShortSequence);
}

unsigned char IsMainProfile(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("IsMainProfile");
  INDIRECT(1); ADD(1); BRANCH(1);
  FLC_sub_end();

  return (pIcsInfo->Profile == ProfileMain);
}

int GetProfile(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetProfile");
  INDIRECT(1);
  FLC_sub_end();

  return pIcsInfo->Profile;
}

char GetWindowShape(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetWindowShape");
  INDIRECT(1);
  FLC_sub_end();

  return pIcsInfo->WindowShape;
}

char GetWindowSequence(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetWindowSequence");
  INDIRECT(1);
  FLC_sub_end();

  return pIcsInfo->WindowSequence;
}

int GetWindowsPerFrame(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetWindowsPerFrame");
  INDIRECT(1); ADD(1); BRANCH(1);
  FLC_sub_end();

  return (pIcsInfo->WindowSequence == EightShortSequence) ? 8 : 1;
}

char GetWindowGroups(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetWindowGroups");
  INDIRECT(1);
  FLC_sub_end();

  return pIcsInfo->WindowGroups;
}

char GetWindowGroupLength(CIcsInfo *pIcsInfo, int index)
{
  FLC_sub_start("GetWindowGroupLength");
  INDIRECT(1);
  FLC_sub_end();

  return pIcsInfo->WindowGroupLength[index];
}

char *GetWindowGroupLengthTable(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetWindowGroupLengthTable");
  INDIRECT(1);
  FLC_sub_end();

  return pIcsInfo->WindowGroupLength;
}

char GetScaleFactorBandsTransmitted(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetScaleFactorBandsTransmitted");
  INDIRECT(1);
  FLC_sub_end();

  return pIcsInfo->MaxSfBands;
}

char GetScaleFactorBandsTotal(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetScaleFactorBandsTotal");
  INDIRECT(1);
  FLC_sub_end();

  return pIcsInfo->TotalSfBands;
}

int SamplingRateFromIndex(int index)
{
  FLC_sub_start("SamplingRateFromIndex");
  INDIRECT(1);
  FLC_sub_end();

  return SamplingRateInfoTable[index].SamplingFrequency;
}

int GetSamplingFrequency(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetSamplingFrequency");
  INDIRECT(1); FUNC(1);
  FLC_sub_end();

  return SamplingRateFromIndex(pIcsInfo->SamplingRateIndex);
}

char GetMaximumTnsBands(CIcsInfo *pIcsInfo)
{
  int idx = IsLongBlock(pIcsInfo) ? 0 : 1;

  FLC_sub_start("GetMaximumTnsBands");
  FUNC(1); MOVE(1); /* counting previous operation */

  INDIRECT(1); /* counting post operation */
  FLC_sub_end();

  return tns_max_bands_tbl[pIcsInfo->SamplingRateIndex][idx];
}

const short *GetScaleFactorBandOffsets(CIcsInfo *pIcsInfo)
{
  FLC_sub_start("GetScaleFactorBandOffsets");

  FUNC(1); BRANCH(1);
  if (IsLongBlock(pIcsInfo))
  {
    INDIRECT(1);
    FLC_sub_end();
    return SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].ScaleFactorBands_Long;
  }
  else
  {
    INDIRECT(1);
    FLC_sub_end();
    return SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].ScaleFactorBands_Short;
  }
}

void IcsReset(CIcsInfo *pIcsInfo, CStreamInfo *pStreamInfo)
{
  FLC_sub_start("IcsReset");

  INDIRECT(2); MOVE(1);
  pIcsInfo->Valid = 0;
  pIcsInfo->TotalSfBands = 0;

  INDIRECT(4); MOVE(2);
  pIcsInfo->SamplingRateIndex = pStreamInfo->SamplingRateIndex;
  pIcsInfo->Profile = pStreamInfo->Profile;

  FLC_sub_end();
}

int IcsRead(HANDLE_BIT_BUF bs,
             CIcsInfo *pIcsInfo)
{
  int i;
  char mask;
  char PredictorDataPresent;
  int ErrorStatus = AAC_DEC_OK;
  
  FLC_sub_start("IcsRead");

  MOVE(1); /* counting previous operation */

  INDIRECT(1); FUNC(2); STORE(1);
  pIcsInfo->IcsReservedBit = (char) GetBits(bs,1);

  INDIRECT(1); FUNC(2); STORE(1);
  pIcsInfo->WindowSequence = (char) GetBits(bs,2);

  INDIRECT(1); FUNC(2); STORE(1);
  pIcsInfo->WindowShape = (char) GetBits(bs,1);

  FUNC(1); BRANCH(1);
  if (IsLongBlock(pIcsInfo))
  {
    INDIRECT(2); MOVE(1);
    pIcsInfo->TotalSfBands = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].NumberOfScaleFactorBands_Long;

    INDIRECT(1); FUNC(2); STORE(1);
    pIcsInfo->MaxSfBands = (char) GetBits(bs,6);
    
    FUNC(2); BRANCH(1);
    if ((PredictorDataPresent = (char) GetBits(bs,1)))
    {
      FLC_sub_end();
      return (AAC_DEC_PREDICTION_NOT_SUPPORTED_IN_LC_AAC);
    }

    INDIRECT(2); MOVE(2);
    pIcsInfo->WindowGroups = 1;
    pIcsInfo->WindowGroupLength[0] = 1;
  }
  else
  {
    INDIRECT(2); MOVE(1);
    pIcsInfo->TotalSfBands = SamplingRateInfoTable[pIcsInfo->SamplingRateIndex].NumberOfScaleFactorBands_Short;

    INDIRECT(1); FUNC(2); STORE(1);
    pIcsInfo->MaxSfBands = (char) GetBits(bs,4);

    INDIRECT(1); FUNC(2); STORE(1);
    pIcsInfo->ScaleFactorGrouping = (char) GetBits(bs,7);

    INDIRECT(1); MOVE(1);
    pIcsInfo->WindowGroups = 0 ;

    PTR_INIT(1); /* pIcsInfo->WindowGroupLength[] */
    LOOP(1);
    for (i=0; i < 7; i++)
    {
      ADD(1); SHIFT(1);
      mask = 1 << (6 - i);

      MOVE(1);
      pIcsInfo->WindowGroupLength[i] = 1;

      INDIRECT(1); LOGIC(1); BRANCH(1);
      if (pIcsInfo->ScaleFactorGrouping & mask)
      {
        INDIRECT(1); ADD(1); STORE(1);
        pIcsInfo->WindowGroupLength[pIcsInfo->WindowGroups]++;
      }
      else
      {
        INDIRECT(1); ADD(1); STORE(1);
        pIcsInfo->WindowGroups++;
      }
    }

    /* loop runs to i < 7 only */
    INDIRECT(1); MOVE(1);
    pIcsInfo->WindowGroupLength[7] = 1;

    INDIRECT(1); ADD(1); STORE(1);
    pIcsInfo->WindowGroups++;
  }

  INDIRECT(1); MOVE(1);
  pIcsInfo->Valid = 1;

  FLC_sub_end();

  return ErrorStatus;
}

