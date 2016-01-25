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

 $Header: /data/cvs/release/FloatFR_sbrdeclib/src/Attic/hybrid.h,v 1.2.8.4 2004/02/17 15:36:17 jr Exp $

*******************************************************************************/
/*
  \file
  \brief  Hybrid Filter Bank header file.
  \author Per Ekstrand
*/

#ifndef _HYBRID_H
#define _HYBRID_H

#include "sbr_const.h"

#define HYBRID_FILTER_LENGTH  13
#define HYBRID_FILTER_DELAY    6

typedef enum {

  HYBRID_2_REAL = 2,
  HYBRID_4_CPLX = 4,
  HYBRID_8_CPLX = 8

} HYBRID_RES;

typedef struct
{
  int   nQmfBands;
  int   *pResolution;
  int   qmfBufferMove;

  float *pWorkReal;
  float *pWorkImag;

  float **mQmfBufferReal;
  float **mQmfBufferImag;
  float *mTempReal;
  float *mTempImag;

} HYBRID;

typedef HYBRID *HANDLE_HYBRID;

void
HybridAnalysis_dec ( const float **mQmfReal,
                 const float **mQmfImag,
                 float **mHybridReal,
                 float **mHybridImag,
                 HANDLE_HYBRID hHybrid );
void
HybridSynthesis_dec ( const float **mHybridReal,
                  const float **mHybridImag,
                  float **mQmfReal,
                  float **mQmfImag,
                  HANDLE_HYBRID hHybrid );

int
CreateHybridFilterBank_dec ( HANDLE_HYBRID *phHybrid,
                         int noBands,
                         const int *pResolution,
                         float **pPtr);

void
DeleteHybridFilterBank ( HANDLE_HYBRID *phHybrid );



#endif /* _HYBRID_H */

