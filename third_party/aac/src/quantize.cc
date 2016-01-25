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
 
 $Id: quantize.c,v 1.1.2.2 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Quantization submodule $Revision: 1.1.2.2 $
  \author M. Werner
*/
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "minmax.h"
#include "quantize.h"
#include "float.h"
#include "aac_rom_enc.h"
#include "FloatFR.h"

#include "flc.h" /* the 3GPP instrumenting tool */





/*****************************************************************************

    functionname:quantizeLines 
    description: quantizes spectrum lines  
                 quaSpectrum = mdctSpectrum^3/4*2^(-(3/16)*gain)    
    input: global gain, number of lines to process, spectral data         
    output: quantized spectrum

*****************************************************************************/
static void quantizeLines(const int gain,
                          const int noOfLines,
                          const float *mdctSpectrum,
                          signed short *quaSpectrum)
{
  float  quantizer;
  float k = (-0.0946f+ 0.5f)/8192.0f;
  int   line;

  FLC_sub_start("quantizeLines");

  MOVE(1); /* counting previous operation */

  PTR_INIT(2); /* mdctSpectrum[line]
                  quaSpectrum[line]
               */
  LOOP(1);
  for (line = 0; line < noOfLines; line++)
  {
    float tmp  = mdctSpectrum[line];

    MOVE(1); /* counting previous operation */

    MOVE(1);
    k = -0.0946f + 0.5f;

    SHIFT(1); ADD(1); LOGIC(1); INDIRECT(2); MULT(1);
    quantizer=quantTableE[(gain>>4)+8] * quantTableQ[gain & 15];

    BRANCH(1);
    if (tmp < 0.0f) {

      MULT(1); TRANS(1);
      tmp = (float)sqrt(-tmp);

      TRANS(1); MULT(1);
      tmp *= (float)sqrt(tmp); /* x^(3/4) */
      
      ADD(1); MULT(2); STORE(1);
      quaSpectrum[line] = -(int)(k + quantizer * tmp);
    }
    else {
      TRANS(1);
      tmp = (float)sqrt(tmp);

      TRANS(1); MULT(1);
      tmp *= (float)sqrt(tmp); /* x^(3/4) */

      MULT(1); ADD(1); STORE(1);
      quaSpectrum[line] = (int)(k + quantizer * tmp);
    }
  }

  FLC_sub_end();
}


/*****************************************************************************

    functionname: QuantizeSpectrum
    description: quantizes the entire spectrum
    returns:
    input: number of scalefactor bands to be quantized, ...
    output: quantized spectrum

*****************************************************************************/
void QuantizeSpectrum(int sfbCnt,
                      int maxSfbPerGroup,
                      int sfbPerGroup,
                      int *sfbOffset,
                      float *mdctSpectrum,
                      int globalGain,
                      short *scalefactors,
                      short *quantizedSpectrum)
{
  int sfbOffs,sfb;

  FLC_sub_start("QuantizeSpectrum");

  LOOP(1);
  for(sfbOffs=0;sfbOffs<sfbCnt;sfbOffs+=sfbPerGroup)
  {
  PTR_INIT(2); /* scalefactors[]
                  sfbOffset[]
               */
  LOOP(1);
  for (sfb = 0; sfb < maxSfbPerGroup; sfb++)
  {
    int scalefactor = scalefactors[sfbOffs+sfb];

    MOVE(1); /* counting previous operation */

    ADD(2); PTR_INIT(2); FUNC(4);
    quantizeLines(globalGain - scalefactor,
                  sfbOffset[sfbOffs+sfb+1] - sfbOffset[sfbOffs+sfb],
                  mdctSpectrum + sfbOffset[sfbOffs+sfb],
                  quantizedSpectrum + sfbOffset[sfbOffs+sfb]);
  }
  }

  FLC_sub_end();
}


void calcExpSpec(float *expSpec, float *mdctSpectrum, int noOfLines)
{
   int i;
   float tmp;

   FLC_sub_start("calcExpSpec");

   PTR_INIT(2); /* pointers for mdctSpectrum[],
                                expSpec[]
                */
   LOOP(1);
   for (i=0; i<noOfLines; i++) {
      /* x^(3/4) */

      MISC(1);
      tmp = (float)fabs(mdctSpectrum[i]);

      TRANS(1);
      tmp = (float)sqrt(tmp);

      TRANS(1); MULT(1); STORE(1);
      expSpec[i] = (float)(tmp * sqrt(tmp)); 
   }

   FLC_sub_end();
}

/*****************************************************************************

    functionname:calcSfbDist 
    description: quantizes and requantizes lines to calculate distortion
    input:  number of lines to be quantized, ...
    output: distortion

*****************************************************************************/
float calcSfbDist(const float *spec,
                  const float *expSpec,
                  short *quantSpec,
                   int  sfbWidth,
                   int  gain)
{
  int i;
  float dist = 0;
  float k =  -0.0946f + 0.5f;
  float quantizer = quantTableE[(gain>>4)+8]*quantTableQ[gain & 15]; 
  float invQuantizer = invQuantTableE[(gain>>4)+8]*invQuantTableQ[gain & 15];
  
  FLC_sub_start("calcSfbDist");

  MOVE(2); SHIFT(1); INDIRECT(4); MULT(2); /* counting previous operations */

  PTR_INIT(3); /* pointer for quantSpec[],
                              expSpec[],
                              spec[]
               */
  LOOP(1);
  for(i=0;i<sfbWidth;i++){
    float iqval;
    float diff;

    MULT(1); ADD(1);
    quantSpec[i] = (int)(k + quantizer * expSpec[i]);

    ADD(1); BRANCH(1);
    if (quantSpec[i] < MAX_POW4_3_TABLE) {
      INDIRECT(1); MULT(1);
      iqval = pow4_3_tab[quantSpec[i]] * invQuantizer;
    }
    else {
      MISC(1); TRANS(1); MULT(1);
      iqval = (float) ( (pow((float)abs(quantSpec[i]), 4.0f/3.0f)) * invQuantizer );
    }

    MISC(1); ADD(1); 
    diff = (float)fabs(spec[i])-iqval;

    MAC(1);
    dist+= diff * diff;

    }


  FLC_sub_end();

  return dist;
}
