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

 $Id: conceal.h,v 1.2.6.2 2004/02/16 17:53:57 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  independent channel concealment  $Revision: 1.2.6.2 $
  \author Andreas Schneider
*/

#ifndef CONCEAL_H
#define CONCEAL_H

#include "aac_ram_dec.h"

#define MAXSFB  51            /* maximum number of scalefactor bands  */

typedef enum{
  CConcealment_Ok,
  CConcealment_FadeOut,
  CConcealment_Mute,
  CConcealment_FadeIn
} CConcealmentState;

typedef enum {
  CConcealment_NoExpand,
  CConcealment_Expand,
  CConcealment_Compress
} CConcealmentExpandType;


void CConcealment_Init (CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo);

void CConcealment_Apply (CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,
                         CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                         char FrameOk);

#endif /* #ifndef CONCEAL_H */
