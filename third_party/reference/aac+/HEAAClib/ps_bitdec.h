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

 $Id: ps_bitdec.h,v 1.2.8.3 2004/02/17 15:36:17 jr Exp $

*******************************************************************************/
/*
  \file
  \brief  Parametric Stereo bitstream processing and decoding $Revision: 1.2.8.3 $
  \author Jonas Engdegard
*/
#ifndef __PS_BITDEC_H
#define __PS_BITDEC_H

#include "sbr_bitb.h"

struct PS_DEC;

#define EXTENSION_ID_PS_CODING   2

unsigned int
ReadPsData (struct PS_DEC  *h_ps_dec,
            HANDLE_BIT_BUFFER hBitBuf,
            int nBitsLeft);

void
DecodePs(struct PS_DEC *h_ps_dec);


#endif
