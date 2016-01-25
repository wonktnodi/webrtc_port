/****************************************************************************

                      (C) copyright Coding Technologies (2001)
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

 $Id: FFR_aacPLUScheck.h,v 1.1.1.1.8.2 2004/02/17 15:36:18 jr Exp $

*******************************************************************************/
/*!
  \file
  \brief  LSI_aacPRO-specific Payload Extraction $Revision: 1.1.1.1.8.2 $
  \author Thomas Ziegler
*/
#ifndef LSI_AACPROCHECK_H
#define LSI_AACPROCHECK_H

#include "FFR_bitbuffer.h"

void FFRaacplus_checkForPayload(HANDLE_BIT_BUF bs,
                                SBRBITSTREAM *streamSBR,
                                int previous_element);

#endif
