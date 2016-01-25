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
 
 $Id: tns.h,v 1.7.2.7 2004/02/16 21:05:59 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Temporal Noise Shaping $Revision: 1.7.2.7 $
  \author A. Groeschel
*/
#ifndef _TNS_H
#define _TNS_H

#include "psy_const.h"
#include "math.h"

#define TNS_MAX_ORDER       12
#define TNS_MAX_ORDER_SHORT 5
#define FILTER_DIRECTION    0

typedef struct{
  float   threshOn;
  int     lpcStartFreq;
  int     lpcStopFreq;
  float   tnsTimeResolution;
}TNS_CONFIG_TABULATED;


typedef struct {
  char tnsActive;
  int tnsMaxSfb;

  int maxOrder;
  int tnsStartFreq;
  int coefRes;

  TNS_CONFIG_TABULATED confTab;

  float acfWindow[TNS_MAX_ORDER+1];
  int tnsStartBand;
  int tnsStartLine;

  int tnsStopBand;
  int tnsStopLine;

  int lpcStartBand;
  int lpcStartLine;

  int lpcStopBand;
  int lpcStopLine;

  int tnsRatioPatchLowestCb;
  int tnsModifyBeginCb;

  float threshold;

}TNS_CONFIG;


typedef struct {
  char  tnsActive;
  float parcor[TNS_MAX_ORDER];
  float predictionGain;
} TNS_SUBBLOCK_INFO;

typedef struct{
  TNS_SUBBLOCK_INFO subBlockInfo[TRANS_FAC];
} TNS_DATA_SHORT;

typedef struct{
  TNS_SUBBLOCK_INFO subBlockInfo;
} TNS_DATA_LONG;

/*
  can be implemented as union
*/
typedef struct{
  TNS_DATA_LONG Long;
  TNS_DATA_SHORT Short;
}TNS_DATA_RAW;

typedef struct{
  int numOfSubblocks;
  TNS_DATA_RAW dataRaw;
}TNS_DATA;

typedef struct{
  char tnsActive[TRANS_FAC];
  char coefRes[TRANS_FAC];
  int length[TRANS_FAC];
  int order[TRANS_FAC];
  /* for Long: length TNS_MAX_ORDER (12 for LC) is required -> 12 */
  /* for Short: length TRANS_FAC*TNS_MAX_ORDER_SHORT (only 5 for short LC) is required -> 8*5=40 */
  int coef[TRANS_FAC*TNS_MAX_ORDER_SHORT];
}TNS_INFO;

#endif /* _TNS_H */
