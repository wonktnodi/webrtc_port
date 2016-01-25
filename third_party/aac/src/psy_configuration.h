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
 
 $Id: psy_configuration.h,v 1.2.2.3 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Psychoaccoustic configuration $Revision: 1.2.2.3 $
  \author M. Werner
*/
#ifndef _PSY_CONFIGURATION_H
#define _PSY_CONFIGURATION_H

#include "psy_const.h"
#include "tns_enc.h"

typedef struct{

  int sfbCnt;
  int sfbActive;
  int sfbOffset[MAX_SFB_LONG+1];
  
  float sfbThresholdQuiet[MAX_SFB_LONG];

  float maxAllowedIncreaseFactor;   /* preecho control */
  float minRemainingThresholdFactor;

  int   lowpassLine;
  float clipEnergy;                 /* for level dependend tmn */

  float ratio;
  float sfbMaskLowFactor[MAX_SFB_LONG];
  float sfbMaskHighFactor[MAX_SFB_LONG];

  float sfbMaskLowFactorSprEn[MAX_SFB_LONG];
  float sfbMaskHighFactorSprEn[MAX_SFB_LONG];


  float sfbMinSnr[MAX_SFB_LONG];

  TNS_CONFIG tnsConf;

} PSY_CONFIGURATION_LONG;


typedef struct{

  int sfbCnt;
  int sfbActive;
  int sfbOffset[MAX_SFB_SHORT+1];

  float sfbThresholdQuiet[MAX_SFB_SHORT];

  float maxAllowedIncreaseFactor;   /* preecho control */
  float minRemainingThresholdFactor;

  int   lowpassLine;
  float clipEnergy;                 /* for level dependend tmn */

  float ratio;
  float sfbMaskLowFactor[MAX_SFB_SHORT];
  float sfbMaskHighFactor[MAX_SFB_SHORT];

  float sfbMaskLowFactorSprEn[MAX_SFB_SHORT];
  float sfbMaskHighFactorSprEn[MAX_SFB_SHORT];


  float sfbMinSnr[MAX_SFB_SHORT];

  TNS_CONFIG tnsConf;

} PSY_CONFIGURATION_SHORT;


int InitPsyConfiguration(long bitrate,
                         long samplerate,
                         int  bandwidth,
                         PSY_CONFIGURATION_LONG *psyConf);

int InitPsyConfigurationShort(long bitrate,
                              long samplerate,
                              int  bandwidth,
                              PSY_CONFIGURATION_SHORT *psyConf);

#endif /* _PSY_CONFIGURATION_H */



