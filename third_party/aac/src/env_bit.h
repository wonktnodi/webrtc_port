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

 $Id: env_bit.h,v 1.1.1.1.2.2 2004/02/17 10:21:21 jr Exp $

*******************************************************************************/
/*!
  \file
  \brief  Remaining SBR Bit Writing Routines
  \author Holger Hoerich <hrc@codingtechnologies.de>
*/

#ifndef BIT_ENV_H
#define BIT_ENV_H

#include "FFR_bitbuffer.h"

#define SBR_CRC_POLY     (0x0233)
#define SBR_CRC_MASK     (0x0200)
#define SBR_CRC_RANGE    (0x03FF)
#define SBR_CRC_MAXREGS     1
#define SBR_CRCINIT      (0x0)

#define AAC_SI_FIL_SBR             13
#define AAC_SI_FIL_SBR_CRC         14


#define SI_SBR_CRC_ENABLE_BITS                  0
#define SI_SBR_CRC_BITS                        10





#define  SI_ID_BITS_AAC             3
#define  SI_FILL_COUNT_BITS         4
#define  SI_FILL_ESC_COUNT_BITS     8
#define  SI_FILL_EXTENTION_BITS     4
#define  ID_FIL                     6






struct COMMON_DATA;

void InitSbrBitstream(struct COMMON_DATA  *hCmonData,
                      unsigned char *memoryBase,
                      int            memorySize,
                      int CRCActive);

void
AssembleSbrBitstream(struct COMMON_DATA  *hCmonData);





#endif /* #ifndef BIT_ENV_H */
