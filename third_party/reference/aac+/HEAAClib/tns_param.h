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
 
 $Id: tns_param.h,v 1.1.1.1.2.2 2004/02/16 21:05:59 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Temporal Noise Shaping parameters $Revision: 1.1.1.1.2.2 $
  \author A. Groeschel
*/
#ifndef _TNS_PARAM_H
#define _TNS_PARAM_H

#include "tns_enc.h"

typedef struct{
  int samplingRate;
  int maxBandLong;
  int maxBandShort;
}TNS_MAX_TAB_ENTRY;

typedef struct{
  int bitRateFrom;
  int bitRateTo;
  const TNS_CONFIG_TABULATED *paramMono_Long;  /* contains TNS parameters */
  const TNS_CONFIG_TABULATED *paramMono_Short;
  const TNS_CONFIG_TABULATED *paramStereo_Long;
  const TNS_CONFIG_TABULATED *paramStereo_Short;
}TNS_INFO_TAB;

int GetTnsParam(TNS_CONFIG_TABULATED *tnsConfigTab, 
                int bitRate, int channels, int blockType);

void GetTnsMaxBands(int samplingRate, int blockType, int* tnsMaxSfb);

#endif /* _TNS_PARAM_H */
