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

 $Id: ps_bitenc.h,v 1.2.2.5 2004/02/17 10:21:23 jr Exp $

*******************************************************************************/
/*!
  \file
  \brief  Parametric stereo bitstream encoder $Revision: 1.2.2.5 $
  \author Jonas Engdegard, Andreas Ehret
*/
#ifndef __PS_BITENC_H
#define __PS_BITENC_H

#include "FFR_bitbuffer.h"
#include "sbr_main.h"

#define CODE_BOOK_LAV_IID 14
#define CODE_BOOK_LAV_ICC 7
#define NO_IID_STEPS                    ( 7 )
#define NO_ICC_STEPS                    ( 8 )


struct PS_ENC;

int WritePsData (struct PS_ENC*  h_ps_e,
                 int                bHeaderActive);

int AppendPsBS (struct PS_ENC*  h_ps_e,
                HANDLE_BIT_BUF     hBitstream,
                HANDLE_BIT_BUF     hBitstreamPrev,
                int                *sbrHdrBits);

#endif
