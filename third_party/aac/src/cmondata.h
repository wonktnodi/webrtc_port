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

 $Id: cmondata.h,v 1.1.1.1.2.4 2004/04/01 13:14:16 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  Core Coder's and SBR's shared data structure definition $Revision: 1.1.1.1.2.4 $
  \author Markus Werner
*/
#ifndef __SBR_CMONDATA_H
#define __SBR_CMONDATA_H

#include "FFR_bitbuffer.h"


struct COMMON_DATA {
  int                   sbrHdrBits;
  int                   sbrCrcLen;
  int                   sbrDataBits;
  int                   sbrFillBits;
  struct BIT_BUF        sbrBitbuf;
  struct BIT_BUF        tmpWriteBitbuf;
  int                   sbrNumChannels;
  struct BIT_BUF        sbrBitbufPrev;

};

typedef struct COMMON_DATA *HANDLE_COMMON_DATA;



#endif
