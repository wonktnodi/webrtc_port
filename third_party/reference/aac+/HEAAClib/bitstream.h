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

 $Id: bitstream.h,v 1.1.1.1.6.2 2004/02/16 15:52:53 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  Bitstream routines, $Revision: 1.1.1.1.6.2 $
  \author zerok 
*/

#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "aacdecoder.h"
#include "FFR_bitbuffer.h"

unsigned long GetBits(HANDLE_BIT_BUF hBitBuf,
                      int nBits);

void PushBack(HANDLE_BIT_BUF bs, int nBits);

void ByteAlign(HANDLE_BIT_BUF hBitBuf,long *pByteAlignBits);


#endif /* #ifndef BITSTREAM_H */

