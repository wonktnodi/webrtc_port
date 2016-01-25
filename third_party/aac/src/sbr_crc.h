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

 $Id: sbr_crc.h,v 1.1.1.1.6.2 2004/02/17 15:36:17 jr Exp $

*******************************************************************************/
/*
  \file
  \brief  CRC checking routines $Revision: 1.1.1.1.6.2 $
  \author Roland Bitto
*/
#ifndef __SBR_CRC_H
#define __SBR_CRC_H

#include "sbr_bitb.h"

#define SBR_CRC_POLY                  0x0233
#define SBR_CRC_MASK                  0x0200
#define SBR_CRC_START                 0x0000
#define SBR_CRC_RANGE                 0x03FF

typedef struct
{
  unsigned short crcState;
  unsigned short crcMask;
  unsigned short crcPoly;
}
CRC_BUFFER;

typedef CRC_BUFFER *HANDLE_CRC;

int SbrCrcCheck (HANDLE_BIT_BUFFER hBitBuf,
                 long NrCrcBits);


#endif
