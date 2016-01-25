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

 $Id: pulsedata.c,v 1.1.2.4 2004/02/16 17:53:57 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  pulse data tool  $Revision: 1.1.2.4 $
  \author zerok 
*/

#include "block.h"
#include "FFR_bitbuffer.h"

#include "flc.h" /* the 3GPP instrumenting tool */

/*
  The function reads the elements for pulse data from
  the bitstream.
*/
void CPulseData_Read(HANDLE_BIT_BUF bs,     /*!< pointer to bitstream */
                     CPulseData *PulseData) /*!< pointer to pulse data side info */
{
  int i;

  FLC_sub_start("CPulseData_Read");

  FUNC(2); INDIRECT(1); STORE(1); BRANCH(1);
  if ((PulseData->PulseDataPresent = (char) GetBits(bs,1)))
  {
    FUNC(2); INDIRECT(2); STORE(1);
    PulseData->NumberPulse = (char) GetBits(bs,2);

    FUNC(2); INDIRECT(2); STORE(1);
    PulseData->PulseStartBand = (char) GetBits(bs,6);

    PTR_INIT(2); /* PulseData->PulseOffset[i]
                    PulseData->PulseAmp[i]
                 */
    LOOP(1);
    for (i=0; i<=PulseData->NumberPulse; i++)
    {
      FUNC(2); STORE(1);
      PulseData->PulseOffset[i] = (char) GetBits(bs,5);

      FUNC(2); STORE(1);
      PulseData->PulseAmp[i] = (char) GetBits(bs,4);
    }
  }

  FLC_sub_end();
}

/*
  The function applies the pulse data to the
  specified spectral lines.
*/
void CPulseData_Apply(CPulseData *PulseData,               /*!< pointer to pulse data side info */
                      const short *pScaleFactorBandOffsets, /*!< pointer to scalefactor band offsets */
                      int *coef)                           /*!< pointer to spectrum */
{
  int i,k;

  FLC_sub_start("CPulseData_Apply");

  INDIRECT(1); BRANCH(1);
  if (PulseData->PulseDataPresent)
  {
    INDIRECT(1); MOVE(1);
    k = pScaleFactorBandOffsets[PulseData->PulseStartBand];
  
    PTR_INIT(2); /* PulseData->PulseOffset[i]
                    PulseData->PulseAmp[i]
                 */
    LOOP(1);
    for (i=0; i<=PulseData->NumberPulse; i++)
    {
      ADD(1);
      k += PulseData->PulseOffset[i];

      INDIRECT(1); BRANCH(1); ADD(1); STORE(1);
      if (coef [k] > 0) coef[k] += PulseData->PulseAmp[i];
      else              coef[k] -= PulseData->PulseAmp[i];
    }
  }

  FLC_sub_end();
}
