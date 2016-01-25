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

 $Id: channel.h,v 1.3.6.5 2004/02/16 15:52:53 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  channel info  $Revision: 1.3.6.5 $
  \author zerok
*/

#ifndef CHANNEL_H
#define CHANNEL_H

#include "block.h"
#include "FFR_bitbuffer.h"

enum
{
  ID_SCE = 0,
  ID_CPE,
  ID_CCE,
  ID_LFE,
  ID_DSE,
  ID_PCE,
  ID_FIL,
  ID_END,
};

enum
{
  L = 0,
  R = 1,
  Channels = 2
};

int ReadICS(HANDLE_BIT_BUF bs,
            CAacDecoderChannelInfo *pAacDecoderChannelInfo);

int CSingleChannelElement_Read(HANDLE_BIT_BUF bs,
                              CAacDecoderChannelInfo *pAacDecoderChannelInfo[Channels],
                              CStreamInfo *pStreamInfo);

int CChannelPairElement_Read(HANDLE_BIT_BUF bs,
                             CAacDecoderChannelInfo *pAacDecoderChannelInfo[Channels],
                             CStreamInfo *pStreamInfo);

void CSingleChannelElement_Decode(AACDECODER self,CAacDecoderChannelInfo *pAacDecoderChannelInfo[Channels]);

void CChannelPairElement_Decode(AACDECODER self,CAacDecoderChannelInfo *pAacDecoderChannelInfo[Channels]);

void MapMidSideMaskToPnsCorrelation (CAacDecoderChannelInfo *pAacDecoderChannelInfo[Channels]);

#endif /* #ifndef CHANNEL_H */
