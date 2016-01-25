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

 $Id: sbr_bitb.c,v 1.1.2.2 2004/02/17 15:36:17 jr Exp $

******************************************************************************/
/*
  \file
  \brief  Bitbuffer management $Revision: 1.1.2.2 $
  \author Markus Werner
*/
#include <assert.h>
#include "sbr_bitb.h"

#include "flc.h" /* the 3GPP instrumenting tool */

/*
 \brief     Reads n bits from Bitbuffer

 \return    bits
*/
unsigned long
getbits (HANDLE_BIT_BUFFER hBitBuf,
         int n)
{
  unsigned long ret_value = 0;

  assert(n <= 32);

  FLC_sub_start("getbits");

  MOVE(1); /* counting previous operation */

  LOOP(1);
  while (n>8) {

    ADD(1);
    n -= 8;

    FUNC(2); SHIFT(1); ADD(1);
    ret_value += getbits(hBitBuf,8) << n;
  }

  INDIRECT(1); ADD(1); BRANCH(1);
  if (hBitBuf->buffered_bits <= 8) {

    INDIRECT(2); SHIFT(1); LOGIC(1); STORE(1);
    hBitBuf->buffer_word = (hBitBuf->buffer_word << 8) | *hBitBuf->char_ptr++;

    ADD(1); STORE(1);
    hBitBuf->buffered_bits += 8;
  }

  ADD(1); STORE(1);
  hBitBuf->buffered_bits -= n;

  SHIFT(2); ADD(2); LOGIC(1);
  ret_value +=
    (hBitBuf->buffer_word >> hBitBuf->buffered_bits) & ((1 << n) - 1);


  INDIRECT(1); ADD(1); STORE(1);
  hBitBuf->nrBitsRead += n;

  FLC_sub_end();

  return (ret_value);
}


/*
  \brief       Initialize variables for reading the bitstream buffer
*/
void
initBitBuffer (HANDLE_BIT_BUFFER hBitBuf,
	       unsigned char *start_ptr,
               unsigned long bufferLen)
{
  FLC_sub_start("initBitBuffer");

  INDIRECT(5); MOVE(5);
  hBitBuf->char_ptr = start_ptr;
  hBitBuf->buffer_word = 0;
  hBitBuf->buffered_bits = 0;
  hBitBuf->nrBitsRead = 0;
  hBitBuf->bufferLen = bufferLen;

  FLC_sub_end();
}


/*
  \brief   Copy the bitbuffer state to a second bitbuffer instance

*/
void
CopyBitbufferState (HANDLE_BIT_BUFFER hBitBuf,
                    HANDLE_BIT_BUFFER hBitBufDest)
{

  FLC_sub_start("CopyBitbufferState");

  INDIRECT(10); MOVE(5);
  hBitBufDest->char_ptr = hBitBuf->char_ptr;
  hBitBufDest->buffer_word = hBitBuf->buffer_word;
  hBitBufDest->buffered_bits = hBitBuf->buffered_bits;
  hBitBufDest->nrBitsRead = hBitBuf->nrBitsRead;
  hBitBufDest->bufferLen = hBitBuf->bufferLen;

  FLC_sub_end();
}


/*
  \brief       returns number bit read since initialisation

  \return      number bit read since initialisation
*/
unsigned long
GetNrBitsRead (HANDLE_BIT_BUFFER hBitBuf)
{
  FLC_sub_start("GetNrBitsRead");
  INDIRECT(1);
  FLC_sub_end();

  return (hBitBuf->nrBitsRead);
}


/*
  \brief       returns number bits available in bit buffer

  \return      number bits available in bit buffer
*/
long
GetNrBitsAvailable (HANDLE_BIT_BUFFER hBitBuf)
{
  FLC_sub_start("GetNrBitsAvailable");
  INDIRECT(2); ADD(1);
  FLC_sub_end();

  return (hBitBuf->bufferLen - hBitBuf->nrBitsRead);
}
