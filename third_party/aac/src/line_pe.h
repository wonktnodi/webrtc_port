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
 
 $Id: line_pe.h,v 1.2.2.1 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  Perceptual entropie module $Revision: 1.2.2.1 $
  \author M. Werner
*/
#ifndef __LINE_PE_H
#define __LINE_PE_H


#include "psy_const.h" 


typedef struct {
   /* these two are calculated by prepareSfbPe */
   float sfbLdEnergy[MAX_GROUPED_SFB];    /* log(sfbEnergy)/log(2) */
   float  sfbNLines[MAX_GROUPED_SFB];         /* number of relevant lines in sfb */
   /* the rest is calculated by calcSfbPe */
   float sfbPe[MAX_GROUPED_SFB];           /* pe for each sfb */
   float sfbConstPart[MAX_GROUPED_SFB];    /* constant part for each sfb */
   float     sfbNActiveLines[MAX_GROUPED_SFB];   /* number of active lines in sfb */
   float pe;                               /* sum of sfbPe */
   float constPart;                        /* sum of sfbConstPart */
   float nActiveLines;                         /* sum of sfbNActiveLines */
} PE_CHANNEL_DATA;


void prepareSfbPe(PE_CHANNEL_DATA *peChanData,
                  const float *sfbEnergy,
                  const float *sfbThreshold,
                  const float  *sfbFormFactor,
                  const int     *sfbOffset,
                  const int     sfbCnt,
                  const int     sfbPerGroup,
                  const int     maxSfbPerGroup);

void calcSfbPe(PE_CHANNEL_DATA *peChanData,
               const float *sfbEnergy,
               const float *sfbThreshold,
               const int    sfbCnt,
               const int    sfbPerGroup,
               const int    maxSfbPerGroup);

#endif 
