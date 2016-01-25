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
 
 $Id: spreading.c,v 1.1.2.2 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Spreading of energy and weighted tonality $Revision: 1.1.2.2 $
  \author M. Werner
*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "spreading.h"
#include "minmax.h"

#include "flc.h" /* the 3GPP instrumenting tool */



void SpreadingMax(const int    pbCnt,
                  const float *maskLowFactor,
                  const float *maskHighFactor,
                  float       *pbSpreadedEnergy)
{
   int i;

   FLC_sub_start("SpreadingMax");

   /* slope to higher frequencies */
   PTR_INIT(2); /* pointers for pbSpreadedEnergy[],
                                maskHighFactor[]
                */
   LOOP(1);
   for (i=1; i<pbCnt; i++) {

      MULT(1); ADD(1); BRANCH(1); MOVE(1);
      pbSpreadedEnergy[i] = max(pbSpreadedEnergy[i],
                                maskHighFactor[i] * pbSpreadedEnergy[i-1]);
   }

   /* slope to lower frequencies */
   PTR_INIT(2); /* pointers for pbSpreadedEnergy[],
                                maskLowFactor[]
                */
   LOOP(1);
   for (i=pbCnt-2; i>=0; i--) {

      MULT(1); ADD(1); BRANCH(1); MOVE(1);
      pbSpreadedEnergy[i] = max(pbSpreadedEnergy[i],
                                maskLowFactor[i] * pbSpreadedEnergy[i+1]);
   }

   FLC_sub_end();

}
