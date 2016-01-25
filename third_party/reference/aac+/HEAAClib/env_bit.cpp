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

 $Id: env_bit.c,v 1.1.2.2 2004/02/17 10:21:21 jr Exp $

*******************************************************************************/
/*!
  \file
  \brief  Remaining SBR Bit Writing Routines
  \author Holger Hoerich <hrc@codingtechnologies.de>
*/

#include <stdlib.h>
#include <string.h>
#include "env_bit.h"
#include "cmondata.h"

#include "flc.h" /* the 3GPP instrumenting tool */

#ifndef min
#define min(a,b) ( a < b ? a:b)
#endif

#ifndef max
#define max(a,b) ( a > b ? a:b)
#endif

/* ***************************** crcAdvance **********************************/
/**
 * @brief    updates crc data register
 *
 */
static void crcAdvance(unsigned short crcPoly,
                       unsigned short crcMask,
                       unsigned short *crc,
                       unsigned long bValue,
                       int           bBits
                )
{
  int i;

  FLC_sub_start("crcAdvance");

  ADD(1); LOOP(1);
  for (i=bBits-1; i>=0; i--)
  {
    unsigned short flag = (*crc) & crcMask ? 1:0;

    LOGIC(1); BRANCH(1); MOVE(1); /* counting previous operation */

    SHIFT(1); LOGIC(1); BRANCH(1); /* .. ? */ LOGIC(1);
    flag ^= (bValue & (1<<i) ? 1 : 0);

    SHIFT(1); STORE(1);
    (*crc)<<=1;

    BRANCH(1);
    if(flag)
    {
      LOGIC(1); STORE(1);
      (*crc) ^= crcPoly;
    }
  }

  FLC_sub_end();
}


/* ***************************** InitSbrBitstream **********************************/
/**
 * @brief    Inittialisation of sbr bitstream, write of dummy header and CRC
 * @return   none
 *
 */

void InitSbrBitstream(HANDLE_COMMON_DATA  hCmonData,
                      unsigned char *memoryBase,
                      int            memorySize,
                      int CRCActive)
{
  FLC_sub_start("InitSbrBitstream");

  INDIRECT(1); PTR_INIT(1); FUNC(3);
  ResetBitBuf(&hCmonData->sbrBitbuf,memoryBase,memorySize);

  INDIRECT(2); MOVE(1);
  hCmonData->tmpWriteBitbuf = hCmonData->sbrBitbuf;

  INDIRECT(1); PTR_INIT(1); FUNC(3);
  WriteBits (&hCmonData->sbrBitbuf, 0, SI_FILL_EXTENTION_BITS);

  BRANCH(1);
  if(CRCActive)
  {
    INDIRECT(1); PTR_INIT(1); FUNC(3);
    WriteBits (&hCmonData->sbrBitbuf, 0,SI_SBR_CRC_BITS);
  }



  FLC_sub_end();
}


/* ************************** AssembleSbrBitstream *******************************/
/**
 * @brief    Formats the SBR payload
 * @return   nothing
 *
 */

void
AssembleSbrBitstream( HANDLE_COMMON_DATA  hCmonData)
{

  unsigned short crcReg =  SBR_CRCINIT;
  int numCrcBits,i;
  int sbrLoad=0;

  FLC_sub_start("AssembleSbrBitstream");

  MOVE(2); /* counting previous operations */

  BRANCH(1);
  if ( hCmonData==NULL )
  {
    FLC_sub_end();
    return;
  }


  INDIRECT(2); ADD(1);
  sbrLoad = hCmonData->sbrHdrBits + hCmonData->sbrDataBits;

  ADD(1);
  sbrLoad += SI_FILL_EXTENTION_BITS;

  INDIRECT(1); BRANCH(1);
  if ( hCmonData->sbrCrcLen )
  {
    ADD(1);
    sbrLoad += SI_SBR_CRC_BITS;
  }

  INDIRECT(1); ADD(1); LOGIC(2); STORE(1);
  hCmonData->sbrFillBits = (8 - (sbrLoad) % 8) % 8;

  ADD(1);
  sbrLoad += hCmonData->sbrFillBits;

  INDIRECT(2); FUNC(3);
  WriteBits(&hCmonData->sbrBitbuf, 0,  hCmonData->sbrFillBits );


  INDIRECT(1); BRANCH(1);
  if ( hCmonData->sbrCrcLen ){
    struct BIT_BUF  tmpCRCBuf = hCmonData->sbrBitbuf;

    MOVE(1); /* counting previous operation */

  PTR_INIT(1); FUNC(2);
  ReadBits (&tmpCRCBuf, SI_FILL_EXTENTION_BITS);

  PTR_INIT(1); FUNC(2);
  ReadBits (&tmpCRCBuf, SI_SBR_CRC_BITS);


    INDIRECT(2); ADD(2);
    numCrcBits = hCmonData->sbrHdrBits + hCmonData->sbrDataBits + hCmonData->sbrFillBits;

    LOOP(1);
    for(i=0;i<numCrcBits;i++){
      int bit;

      PTR_INIT(1); FUNC(2);
      bit=ReadBits(&tmpCRCBuf,1);

      PTR_INIT(1); FUNC(5);
      crcAdvance(SBR_CRC_POLY,SBR_CRC_MASK,&crcReg,bit,1);
    }

    LOGIC(1);
    crcReg &= (SBR_CRC_RANGE);
  }


    INDIRECT(1); BRANCH(1);
    if ( hCmonData->sbrCrcLen ) {

      INDIRECT(1); PTR_INIT(1); FUNC(3);
      WriteBits (&hCmonData->tmpWriteBitbuf, AAC_SI_FIL_SBR_CRC, SI_FILL_EXTENTION_BITS);

      FUNC(3);
      WriteBits (&hCmonData->tmpWriteBitbuf, crcReg,SI_SBR_CRC_BITS);

    }
    else {

      INDIRECT(1); PTR_INIT(1); FUNC(3);
      WriteBits (&hCmonData->tmpWriteBitbuf, AAC_SI_FIL_SBR, SI_FILL_EXTENTION_BITS);
    }




    FLC_sub_end();
 }

