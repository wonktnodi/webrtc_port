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
 
 $Id: interface.c,v 1.1.2.2 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  Interface psychoaccoustic/quantizer $Revision: 1.1.2.2 $
  \author M. Werner
*/
#include "minmax.h"
#include "psy_const.h"
#include "interface.h"

#include "flc.h" /* the 3GPP instrumenting tool */

void BuildInterface(float                  *groupedMdctSpectrum,
                    SFB_THRESHOLD           *groupedSfbThreshold,
                    SFB_ENERGY              *groupedSfbEnergy,
                    SFB_ENERGY              *groupedSfbSpreadedEnergy,
                    const SFB_ENERGY_SUM    sfbEnergySumLR,
                    const SFB_ENERGY_SUM    sfbEnergySumMS,
                    const int               windowSequence,
                    const int               windowShape,
                    const int               groupedSfbCnt,
                    const int              *groupedSfbOffset,
                    const int               maxSfbPerGroup,
                    const float          *groupedSfbMinSnr,
                    const int               noOfGroups,
                    const int              *groupLen,
                    PSY_OUT_CHANNEL        *psyOutCh) // output
{
  int j;
  int grp;
  int mask;

  FLC_sub_start("BuildInterface");

  /*
    copy values to psyOut
  */

  INDIRECT(11); MOVE(8); DIV(1); STORE(1);
  psyOutCh->maxSfbPerGroup = maxSfbPerGroup;
  psyOutCh->sfbCnt         = groupedSfbCnt;
  psyOutCh->sfbPerGroup    = groupedSfbCnt / noOfGroups;
  psyOutCh->windowSequence = windowSequence;
  psyOutCh->windowShape    = windowShape;
  psyOutCh->mdctSpectrum   = groupedMdctSpectrum;
  psyOutCh->sfbEnergy      = groupedSfbEnergy->Long;
  psyOutCh->sfbThreshold   = groupedSfbThreshold->Long;
  psyOutCh->sfbSpreadedEnergy = groupedSfbSpreadedEnergy->Long;
  

  PTR_INIT(2); /* psyOutCh->sfbOffsets[]
                  groupedSfbOffset[]
               */
  LOOP(1);
  for(j=0; j<groupedSfbCnt+1; j++)
  {
    MOVE(1);
    psyOutCh->sfbOffsets[j]=groupedSfbOffset[j];
  }

  PTR_INIT(2); /* psyOutCh->sfbMinSnr[]
                  groupedSfbMinSnr[]
               */
  for(j=0;j<groupedSfbCnt; j++){

    MOVE(1);
    psyOutCh->sfbMinSnr[j] = groupedSfbMinSnr[j];
  }

  /* generate grouping mask */
  MOVE(1);
  mask = 0;

  PTR_INIT(1); /* groupLen[grp] */
  LOOP(1);
  for (grp = 0; grp < noOfGroups; grp++)
  {
    SHIFT(1);
    mask <<= 1;

    LOOP(1);
    for (j=1; j<groupLen[grp]; j++) {

      SHIFT(1); LOGIC(1);
      mask<<=1; 
      mask|=1;
    }
  }
  /* copy energy sums to psy Out for stereo preprocessing */
  
  ADD(1); BRANCH(1);
  if (windowSequence != SHORT_WINDOW) {

    INDIRECT(2); MOVE(2);
    psyOutCh->sfbEnSumLR =  sfbEnergySumLR.Long;
    psyOutCh->sfbEnSumMS =  sfbEnergySumMS.Long;
  }
  else {
    int i;

    INDIRECT(2); MOVE(2);
    psyOutCh->sfbEnSumMS=0;
    psyOutCh->sfbEnSumLR=0;

    PTR_INIT(2); /* sfbEnergySumLR.Short[]
                    sfbEnergySumMS.Short[]
                 */
    LOOP(1);
    for (i=0;i< TRANS_FAC; i++) {

      ADD(2);
      psyOutCh->sfbEnSumLR += sfbEnergySumLR.Short[i];
      psyOutCh->sfbEnSumMS += sfbEnergySumMS.Short[i];
    }
    INDIRECT(2); STORE(2);
  }

  INDIRECT(1); MOVE(1);
  psyOutCh->groupingMask = mask;

  FLC_sub_end();
}
