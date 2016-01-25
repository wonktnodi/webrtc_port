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
 
 $Id: tns_func.h,v 1.6.2.4 2004/02/16 21:05:59 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Temporal Noise Shaping $Revision: 1.6.2.4 $
  \author A. Groeschel
*/
#ifndef _TNS_FUNC_H
#define _TNS_FUNC_H
#include "psy_configuration.h"

int InitTnsConfiguration(int bitrate,
                         long samplerate,
                         int channels,
                         TNS_CONFIG *tnsConfig,
                         PSY_CONFIGURATION_LONG psyConfig,
                         int active);

int InitTnsConfigurationShort(int bitrate,
                              long samplerate,
                              int channels,
                              TNS_CONFIG *tnsConfig,
                              PSY_CONFIGURATION_SHORT psyConfig,
                              int active);

int TnsDetect(TNS_DATA* tnsData,
              TNS_CONFIG tC,
              float* pScratchTns,
              const int sfbOffset[],
              float* spectrum,
              int subBlockNumber,
              int blockType,
              float * sfbEnergy);

void TnsSync(TNS_DATA *tnsDataDest,
             const TNS_DATA *tnsDataSrc,
             const TNS_CONFIG tC,
             const int subBlockNumber,
             const int blockType);

int TnsEncode(TNS_INFO* tnsInfo,
              TNS_DATA* tnsData,
              int numOfSfb,
              TNS_CONFIG tC,
              int lowPassLine,
              float* spectrum,
              int subBlockNumber,
              int blockType);

void ApplyTnsMultTableToRatios(int startCb,
                               int stopCb,
                               float *thresholds);


#endif /* _TNS_FUNC_H */
