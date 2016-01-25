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

 $Id: bitstream.c,v 1.1.2.4 2004/02/16 17:53:57 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  Bitstream routines, $Revision: 1.1.2.4 $
  \author zerok 
*/

#include "aac_ram_dec.h"
#include "FloatFR.h"
#include "FFR_bitbuffer.h"

#include "flc.h" /* the 3GPP instrumenting tool */

/*
  The function loads bits from bitstream buffer.
  
  return:  valid bits
*/
unsigned long GetBits(HANDLE_BIT_BUF hBitBuf,  /*!< pointer to current data in bitstream */
                      int nBits)               /*!< number of bits */
{
  unsigned short tmp;

  FLC_sub_start("GetBits");

  FUNC(2);
  tmp = (unsigned short) ReadBits(hBitBuf,nBits);

  FLC_sub_end();

  return (tmp);
}


/*
  The function rewinds the bitstream pointeers
*/
void PushBack(HANDLE_BIT_BUF hBitBuf, /*!< pointer to current data in bitstream */
              int nBits)              /*!< number of bits */
{
  FLC_sub_start("PushBack");

  MULT(1); FUNC(2);
  WindBitBufferBidirectional(hBitBuf,-nBits);

  FLC_sub_end();
}


/*
  The function applies byte alignement
*/
void ByteAlign(HANDLE_BIT_BUF hBitBuf, /*!< pointer to current data in bitstream */
               long *pByteAlignBits)   /*!< pointer to last state of cntBits */
{
  int alignment;

  FLC_sub_start("ByteAlign");

  INDIRECT(1); ADD(1); LOGIC(1);
  alignment = (*pByteAlignBits - hBitBuf->cntBits) % 8;

  BRANCH(1);
  if (alignment)
  {
    ADD(1); FUNC(2);
    GetBits(hBitBuf,8 - alignment);
  }

  MOVE(1);
  *pByteAlignBits = hBitBuf->cntBits;

  FLC_sub_end();
}
