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

 $Id: imdct.c,v 1.1.2.5 2004/02/16 17:53:57 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  inverse transformation  $Revision: 1.1.2.5 $
  \author zerok 
*/

#include <math.h>
#include "aac_rom_dec.h"
#include "imdct.h"
#include "FloatFR.h"
#include "cfftn.h"

#include "flc.h" /* the 3GPP instrumenting tool */


/*
  The function performs the pre modulation in the inverse
  transformation.
*/
static void 
preModulation(float *x,                /*!< pointer to spectrum */
              int m,                   /*!< number of lines in spectrum */
              const float *sineWindow) /*!< pointer to modulation coefficients */
{
  int i;
  float wre, wim, re1, re2, im1, im2;

  FLC_sub_start("preModulation");

  PTR_INIT(4);  /* pointers for x[2*i], x[m-2-2*i],
                                sineWindow[i*2],
                                sineWindow[m-1-2*i] */
  MULT(1); LOOP(1);
  for (i = 0; i < m/4; i++)
  {
    MOVE(4);
    re1 = x[2*i];
    im2 = x[2*i+1];
    re2 = x[m-2-2*i];
    im1 = x[m-1-2*i];

    MOVE(2);
    wim = sineWindow[i*2];
    wre = sineWindow[m-1-2*i];

    MULT(2); MAC(1); STORE(1);
    x[2*i] = 0.5f * (im1*wim + re1*wre);

    MULT(3); ADD(1); STORE(1);
    x[2*i+1] = 0.5f * (im1*wre - re1*wim);

    MOVE(2);
    wim = sineWindow[m-2-2*i];
    wre = sineWindow[2*i+1];

    MULT(2); MAC(1); STORE(1);   
    x[m-2-2*i] = 0.5f * (im2*wim + re2*wre);

    MULT(3); ADD(1); STORE(1);
    x[m-1-2*i] = 0.5f * (im2*wre - re2*wim);
  }

  FLC_sub_end();
}

/*
  The function performs the post modulation in the inverse
  transformation.
*/
static void 
postModulation(float *x,              /*!< pointer to spectum */
               int m,                 /*!< number of lines in spectrum */
               const float *trigData, /*!< pointer to trigData */
               int step,              /*!< steps */
               int trigDataSize)      /*!< size of trigData */
{
  int i;
  float wre, wim, re1, re2, im1, im2;
  const float *sinPtr = trigData;
  const float *cosPtr = trigData+trigDataSize;

  FLC_sub_start("postModulation");

  MOVE(2); /* previous initialization */

  MOVE(2);
  wim = *sinPtr;
  wre = *cosPtr;

  PTR_INIT(2);  /* pointers for x[2*i], x[m-2-2*i] */
  MULT(1); LOOP(1);
  for (i = 0; i < m/4; i++)
  {
    MOVE(4);
    re1=x[2*i];
    im1=x[2*i+1];
    re2=x[m-2-2*i];
    im2=x[m-1-2*i];

    MULT(1); MAC(1); STORE(1);
    x[2*i] = re1*wre + im1*wim;

    MULT(2); ADD(1); STORE(1);
    x[m-1-2*i] = re1*wim - im1*wre;

    ADD(2);
    sinPtr+=step;
    cosPtr-=step;

    MOVE(2);
    wim=*sinPtr;
    wre=*cosPtr;

    MULT(1); MAC(1); STORE(1);   
    x[m-2-2*i] = re2*wim + im2* wre;

    MULT(2); ADD(1); STORE(1);
    x[2*i+1] = re2*wre - im2* wim;
  }

  FLC_sub_end();
}


/*
  The calculation of the imdct is divided into three steps, the pre modulation,
  the complex fft and the post modulation. The imdct is calculated in-place.
*/
void CLongBlock_InverseTransform(float *pData)      /*!< pointer to input/output data */
{
  int trigDataSize;

  FLC_sub_start("CLongBlock_InverseTransform");

  MOVE(1);
  trigDataSize = 512;

  /* calculate imdct */
  FUNC(3);
  preModulation(pData,FRAME_SIZE,OnlyLongWindowSine);

  FUNC(3);
  CFFTN(pData, trigDataSize, -1);

  FUNC(5);
  postModulation(pData,FRAME_SIZE,trigData,1,trigDataSize);

  FLC_sub_end();
}

/*!
  The calculation of the imdct is divided into three steps, the pre modulation,
  the complex fft and the post modulation. The imdct is calculated in-place.
*/
void CShortBlock_InverseTransform(float *pData)       /*!< pointer to input/output data */
{
  int trigDataSize;

  FLC_sub_start("CShortBlock_InverseTransform");

  trigDataSize = 512;

  /* calculate imdct */
  FUNC(3);
  preModulation(pData,FRAME_SIZE/8,OnlyShortWindowSine);

  FUNC(3);
  CFFTN(pData, trigDataSize/8, -1);

  FUNC(5);
  postModulation(pData,FRAME_SIZE/8,trigData,8,trigDataSize);

  FLC_sub_end();
}
