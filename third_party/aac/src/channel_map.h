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
 
 $Id: channel_map.h,v 1.1.1.1.2.4 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  Channel Mapping  $Revision: 1.1.1.1.2.4 $
  \author A. Groeschel
*/
#ifndef _CHANNEL_MAP_H
#define _CHANNEL_MAP_H

#include "psy_const.h"
#include "qc_data.h"


int InitElementInfo (int nChannels,
                     ELEMENT_INFO *elInfo);

int InitElementBits(ELEMENT_BITS*   elementBits,
                    ELEMENT_INFO elInfo,
                    int bitrateTot,
                    int averageBitsTot,
                    int staticBitsTot);

#endif /* CHANNEL_MAP_H */
