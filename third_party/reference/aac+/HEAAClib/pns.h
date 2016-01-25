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

 $Id: pns.h,v 1.4.6.2 2004/02/16 15:52:53 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  perceptual noise substitution tool  $Revision: 1.4.6.2 $
  \author zerok 
*/

#ifndef PNS_H
#define PNS_H

#include "channelinfo.h"

void CPns_InitPns(CAacDecoderChannelInfo *pAacDecoderChannelInfo);

void CPns_InitInterChannelData(CAacDecoderChannelInfo *pAacDecoderChannelInfo);

int CPns_IsPnsUsed(CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                   const int group,
                   const int band);

void CPns_SetCorrelation(CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                         const int group,
                         const int band);

int CPns_IsCorrelated(CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                      const int group,
                      const int band);

void CPns_Read (CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                HANDLE_BIT_BUF bs,
                const CodeBookDescription *hcb,
                unsigned char global_gain,
                int band,
                int group);

void CPns_Apply (AACDECODER self,CAacDecoderChannelInfo *pAacDecoderChannelInfo[],
                 int channel);

#endif /* #ifndef PNS_H */
