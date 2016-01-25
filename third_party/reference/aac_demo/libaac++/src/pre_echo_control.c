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
 
 $Id: pre_echo_control.c,v 1.1.2.3 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  Pre echo control $Revision: 1.1.2.3 $
  \author M. Werner
*/
#include <math.h>
#include "pre_echo_control.h"

#include "flc.h" /* the 3GPP instrumenting tool */

void InitPreEchoControl(float *pbThresholdNm1,
                        int numPb,
                        float *pbThresholdQuiet)
{
  int pb;

  FLC_sub_start("InitPreEchoControl");

  PTR_INIT(2); /* pbThresholdNm1[]
                  pbThresholdQuiet[]
               */
  LOOP(1);
  for(pb=0;pb<numPb;pb++)
  {
    MOVE(1);
    pbThresholdNm1[pb]=pbThresholdQuiet[pb];
  }

  FLC_sub_end();
}



void PreEchoControl(float *pbThresholdNm1,
                    int     numPb,
                    float  maxAllowedIncreaseFactor,
                    float  minRemainingThresholdFactor,
                    float  *pbThreshold)
{
  int i;
  float tmpThreshold1, tmpThreshold2;

  FLC_sub_start("PreEchoControl");

    PTR_INIT(2); /* pbThresholdNm1[]
                    pbThreshold[]
                 */
    LOOP(1);
    for(i = 0; i < numPb; i++) {
      
      MULT(2);
      tmpThreshold1 = maxAllowedIncreaseFactor * (pbThresholdNm1[i]);
      tmpThreshold2 = minRemainingThresholdFactor * pbThreshold[i];
      
      /* copy thresholds to internal memory */
      MOVE(1);
      pbThresholdNm1[i] = pbThreshold[i];
      
      ADD(1); BRANCH(1);
      if(pbThreshold[i] > tmpThreshold1) {

        MOVE(1);
        pbThreshold[i] = tmpThreshold1;
      }

      ADD(1); BRANCH(1);
      if(tmpThreshold2 > pbThreshold[i]) {

        MOVE(1);
        pbThreshold[i] = tmpThreshold2;
      }
      
    }
  
  FLC_sub_end();
}
