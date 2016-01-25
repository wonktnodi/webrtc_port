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

 $Id: channel.c,v 1.1.2.5 2004/02/16 17:53:57 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  channel info  $Revision: 1.1.2.5 $
  \author zerok
*/

#include "channel.h"
#include "FFR_bitbuffer.h"
#include "conceal.h"
#include "stereo.h"
#include "pns.h"

#include "flc.h" /* the 3GPP instrumenting tool */


/*
  The function reads the bitstream elements of the individual channel stream.
  Depending on window sequence the subroutine for short or long blocks are called.
*/
int ReadICS(HANDLE_BIT_BUF bs,                             /*!< pointer to bitstream */
            CAacDecoderChannelInfo *pAacDecoderChannelInfo /*!< pointer to aac decoder channel info */
            )
{
  int ErrorStatus = AAC_DEC_OK;
  
  FLC_sub_start("ReadICS");

  MOVE(1); /* counting previous operation */

  INDIRECT(1); PTR_INIT(1); /* for &pAacDecoderChannelInfo->IcsInfo */

  /* reads one individual_channel_stream */
  FUNC(2); INDIRECT(1); STORE(1);
  pAacDecoderChannelInfo->RawDataInfo.GlobalGain = (unsigned char) GetBits(bs,8);


  FUNC(1); BRANCH(1);
  if (!IsValid(&pAacDecoderChannelInfo->IcsInfo))
  {
    FUNC(2); ADD(1); BRANCH(1);
    if ((ErrorStatus = IcsRead(bs,&pAacDecoderChannelInfo->IcsInfo)))
    {
      FLC_sub_end();
      return (ErrorStatus);
    }
  }

  FUNC(1);
  if (IsLongBlock(&pAacDecoderChannelInfo->IcsInfo))
  {
    INDIRECT(1); FUNC(3); ADD(1); BRANCH(1);
    if ((ErrorStatus = CLongBlock_Read(bs,pAacDecoderChannelInfo,pAacDecoderChannelInfo->RawDataInfo.GlobalGain)))
    {
      FLC_sub_end();
      return (ErrorStatus);
    }
  }
  else
  {
    FUNC(1);
    CShortBlock_Init(pAacDecoderChannelInfo);

    INDIRECT(1); FUNC(3); ADD(1); BRANCH(1);
    if ((ErrorStatus = CShortBlock_Read(bs,pAacDecoderChannelInfo,pAacDecoderChannelInfo->RawDataInfo.GlobalGain)))
    {
      FLC_sub_end();
      return (ErrorStatus);
    }
  }

  FLC_sub_end();

  return (ErrorStatus);
}

/*
  The function reads the bitstream elements of the single channel element.

  return:  element instance tag
*/
int CSingleChannelElement_Read(HANDLE_BIT_BUF bs,                                 /*!< pointer to bitstream */
                               CAacDecoderChannelInfo *pAacDecoderChannelInfo[2], /*!< pointer to aac decoder channel info */
                               CStreamInfo *pStreamInfo                           /*!< pointer to stream info */
                               )
{
  int ErrorStatus = AAC_DEC_OK;

  FLC_sub_start("CSingleChannelElement_Read");

  MOVE(1); /* counting previous operation */


  FUNC(2); INDIRECT(1); STORE(1);
  pAacDecoderChannelInfo[L]->RawDataInfo.ElementInstanceTag = (char) GetBits(bs,4);
  
  INDIRECT(1); PTR_INIT(1); FUNC(2);
  IcsReset(&pAacDecoderChannelInfo[L]->IcsInfo,pStreamInfo);
  
  INDIRECT(1); FUNC(2); ADD(1); BRANCH(1);
  if (( ErrorStatus = ReadICS(bs,pAacDecoderChannelInfo[L])))
    {
      FLC_sub_end();
      return (ErrorStatus);
    }
  
  FLC_sub_end();

  return (ErrorStatus);
}

/*
  The function reads the bitstream elements of the channel pair element.

  return:  element instance tag
*/
int CChannelPairElement_Read(HANDLE_BIT_BUF bs,                                 /*!< pointer to bitstream */
                             CAacDecoderChannelInfo *pAacDecoderChannelInfo[2], /*!< pointer to aac decoder channel info */
                             CStreamInfo *pStreamInfo                           /*!< pointer to stream info */
                             )
{
  int ErrorStatus = AAC_DEC_OK;
  
  FLC_sub_start("CChannelPairElement_Read");

  MOVE(1); /* counting previous operation */

  FUNC(2); INDIRECT(1); STORE(1);
  pAacDecoderChannelInfo[L]->RawDataInfo.ElementInstanceTag = (char) GetBits(bs,4);

  INDIRECT(2); PTR_INIT(2); FUNC(2);
  IcsReset(&pAacDecoderChannelInfo[L]->IcsInfo,pStreamInfo);
  IcsReset(&pAacDecoderChannelInfo[R]->IcsInfo,pStreamInfo);

  FUNC(2); INDIRECT(1); STORE(1);
  pAacDecoderChannelInfo[L]->RawDataInfo.CommonWindow = (char) GetBits(bs,1);

  INDIRECT(1); BRANCH(1);
  if (pAacDecoderChannelInfo[L]->RawDataInfo.CommonWindow)
  {
    INDIRECT(1); PTR_INIT(1); FUNC(2); ADD(1); BRANCH(1);
    if ((ErrorStatus = IcsRead(bs,&pAacDecoderChannelInfo[L]->IcsInfo)))
    {
      FLC_sub_end();
      return (ErrorStatus);
    }

    INDIRECT(2); MOVE(1);
    pAacDecoderChannelInfo[R]->IcsInfo = pAacDecoderChannelInfo[L]->IcsInfo;

    INDIRECT(2); PTR_INIT(1); FUNC(1); FUNC(1); FUNC(4);
    CJointStereo_Read(bs,
                      pAacDecoderChannelInfo[L]->pJointStereoData,
                      GetWindowGroups(&pAacDecoderChannelInfo[L]->IcsInfo),
                      GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[L]->IcsInfo));
  }

  FUNC(2); ADD(1); BRANCH(1);
  if ((ErrorStatus = ReadICS(bs,pAacDecoderChannelInfo[L])))
  {
    FLC_sub_end();
    return (ErrorStatus);
  }


  FUNC(2); ADD(1); BRANCH(1);
  if ((ErrorStatus = ReadICS(bs,pAacDecoderChannelInfo[R])))
  {
    FLC_sub_end();
    return (ErrorStatus);
  }

  FLC_sub_end();

  return (ErrorStatus);
}

/*
  The function decodes a single channel element.

  return:  none
*/
void CSingleChannelElement_Decode(AACDECODER self,CAacDecoderChannelInfo *pAacDecoderChannelInfo[]  /*!< pointer to aac decoder channel info */
                                  )
{
  FLC_sub_start("CSingleChannelElement_Decode");

  FUNC(2);
  ApplyTools (self, pAacDecoderChannelInfo,L);

  FLC_sub_end();
}

/*
  The function decodes a channel pair element.

  return:  none
*/
void CChannelPairElement_Decode(AACDECODER self,CAacDecoderChannelInfo *pAacDecoderChannelInfo[]  /*!< pointer to aac decoder channel info */
                                )
{
  int channel;

  FLC_sub_start("CChannelPairElement_Decode");

  /* apply ms */
  INDIRECT(1); BRANCH(1);
  if (pAacDecoderChannelInfo[L]->RawDataInfo.CommonWindow) {

    INDIRECT(2); LOGIC(1); BRANCH(1);
    if (pAacDecoderChannelInfo[L]->PnsData.PnsActive || pAacDecoderChannelInfo[R]->PnsData.PnsActive)
    {
      FUNC(1);
      MapMidSideMaskToPnsCorrelation(pAacDecoderChannelInfo);
    }

    INDIRECT(1); PTR_INIT(1); FUNC(1); FUNC(1); FUNC(1); FUNC(1); FUNC(5);
    CJointStereo_ApplyMS(pAacDecoderChannelInfo,
                         GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L]->IcsInfo),
                         GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->IcsInfo),
                         GetWindowGroups(&pAacDecoderChannelInfo[L]->IcsInfo),
                         GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[L]->IcsInfo));
  }

  /* apply intensity stereo */
  INDIRECT(1); PTR_INIT(1); FUNC(1); FUNC(1); FUNC(1); FUNC(1); BRANCH(1); FUNC(5);
  CJointStereo_ApplyIS(pAacDecoderChannelInfo,
                       GetScaleFactorBandOffsets(&pAacDecoderChannelInfo[L]->IcsInfo),
                       GetWindowGroupLengthTable(&pAacDecoderChannelInfo[L]->IcsInfo),
                       GetWindowGroups(&pAacDecoderChannelInfo[L]->IcsInfo),
                       GetScaleFactorBandsTransmitted(&pAacDecoderChannelInfo[L]->IcsInfo),
                       pAacDecoderChannelInfo[L]->RawDataInfo.CommonWindow ? 1 : 0);

  LOOP(1);
  for (channel=0; channel < Channels; channel++)
  {
    FUNC(2);
    ApplyTools (self,pAacDecoderChannelInfo, channel);
  }

  FLC_sub_end();
}

void MapMidSideMaskToPnsCorrelation (CAacDecoderChannelInfo *pAacDecoderChannelInfo[Channels])
{
  int group;
  char band;

  FLC_sub_start("MapMidSideMaskToPnsCorrelation");

  INDIRECT(1); LOOP(1);
  for (group = 0 ; group < pAacDecoderChannelInfo[L]->IcsInfo.WindowGroups; group++) {

    PTR_INIT(1); /* AacDecoderChannelInfo[L]->pJointStereoData->MsUsed[band] */
    INDIRECT(1); LOOP(1);
    for (band = 0 ; band < pAacDecoderChannelInfo[L]->IcsInfo.MaxSfBands; band++) {

      SHIFT(1); LOGIC(1); BRANCH(1);
      if (pAacDecoderChannelInfo[L]->pJointStereoData->MsUsed[band] & (1 << group)) { /* channels are correlated */

        FUNC(3);
        CPns_SetCorrelation(pAacDecoderChannelInfo[L], group, band);

        FUNC(3); FUNC(3); LOGIC(1); BRANCH(1);
        if (CPns_IsPnsUsed(pAacDecoderChannelInfo[L], group, band) &&
            CPns_IsPnsUsed(pAacDecoderChannelInfo[R], group, band))
        {
          LOGIC(1); STORE(1);
          pAacDecoderChannelInfo[L]->pJointStereoData->MsUsed[band] ^= (1 << group);
        }
      }
    }
  }

  FLC_sub_end();
}
