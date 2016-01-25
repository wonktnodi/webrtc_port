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

 $Id: aac_ram.c,v 1.1.2.6 2004/05/30 14:04:23 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  Memory layout  $Revision: 1.1.2.6 $
  \author zerok
*/
#include <stdio.h>
#include "aacdecoder.h"
#include "aac_ram_dec.h"
#include "FloatFR.h"

/*
  Static memory areas, must not be overwritten in other sections of the decoder
*/

//jum modify
/* The structure CAacDecoderStaticChannelInfo contains the static sideinfo which is needed
   for the decoding of one aac channel. Dimension: #AacDecoderChannels */
//CAacDecoderStaticChannelInfo g_AacDecoderStaticChannelInfo[Channels];

//jum modify
/* The structure CStreamInfo contains the streaming information. */
//CStreamInfo g_StreamInfo;


//jum modify
/* Overlap buffer */
//float g_OverlapBuffer[Channels*OverlapBufferSize];

//CConcealmentInfo ConcealmentInfo[Channels];



/*!
  Dynamic memory areas, might be reused in other algorithm sections,
  e.g. the sbr decoder
*/

//jum modify
/* AAC Decoder Workbuffer */
//float g_WorkBufferCore[Channels*MaximumBinsLong];
//extern float TimeDataFloat[4*MaximumBinsLong]; /* allocated in main.c */

#ifdef LP_SBR_ONLY
CAacDecoderChannelInfo AacDecoderChannelInfo[Channels];

CAacDecoderChannelInfo *pAacDecoderChannelInfoInit[Channels] =
{
  &AacDecoderChannelInfo[0],
  &AacDecoderChannelInfo[1]
};
#else
//extern float InterimResult[MaximumBinsLong];

#endif
