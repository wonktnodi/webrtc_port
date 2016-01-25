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
 
 $Id: quantize.h,v 1.4.2.4 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Quantization submodule $Revision: 1.4.2.4 $
  \author M. Werner
*/
#ifndef _QUANTIZE_H_
#define _QUANTIZE_H_

#define MAX_QUANT 8191

void initQuantTables(void);

void QuantizeSpectrum(int sfbCnt, 
                      int maxSfbPerGroup,
                      int sfbPerGroup,
                      int *sfbOffset, float *mdctSpectrum,
                      int globalGain, short *scalefactors,
                      short *quantizedSpectrum);

void calcExpSpec(float *expSpec, float *mdctSpectrum, int noOfLines);

float calcSfbDist(const float *spec,
                  const float *expSpec,
                  short *quantSpec,
                   int  sfbWidth,
                   int  gain);

#endif /* _QUANTIZE_H_ */
