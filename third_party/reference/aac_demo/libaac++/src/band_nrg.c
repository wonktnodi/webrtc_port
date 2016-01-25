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
 
 $Id: band_nrg.c,v 1.1.2.2 2004/02/16 20:02:18 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  calculation of band energies $Revision: 1.1.2.2 $
  \author M. Werner
*/
#include "band_nrg.h"

#include "flc.h" /* the 3GPP instrumenting tool */

void CalcBandEnergy(const float *mdctSpectrum,
                    const int   *bandOffset,
                    const int    numBands,
                    float      *bandEnergy,
                    float       *bandEnergySum) {
  int i, j;

  FLC_sub_start("CalcBandEnergy");

  MOVE(2);
  j = 0;
  *bandEnergySum = 0.0f;
 
  PTR_INIT(3); /* pointers for bandEnergy[],
                               bandOffset[],
                               mdctSpectrum[]
                */
  LOOP(1);
  for(i=0; i<numBands; i++) {

    MOVE(1);
    bandEnergy[i] = 0.0f;

    LOOP(1);
    while (j < bandOffset[i+1]) {

      MAC(1); STORE(1);
      bandEnergy[i] += mdctSpectrum[j] * mdctSpectrum[j];

      j++;
    }
    ADD(1); STORE(1);
    *bandEnergySum += bandEnergy[i];
  }

  FLC_sub_end();
}



void CalcBandEnergyMS(const float *mdctSpectrumLeft,
                      const float *mdctSpectrumRight,
                      const int   *bandOffset,
                      const int    numBands,
                      float      *bandEnergyMid,
                      float       *bandEnergyMidSum,
                      float      *bandEnergySide,
                      float       *bandEnergySideSum) {

  int i, j;

  FLC_sub_start("CalcBandEnergyMS");

  MOVE(3);
  j = 0;
	*bandEnergyMidSum = 0.0f;
	*bandEnergySideSum = 0.0f;

  PTR_INIT(5); /* pointers for bandEnergyMid[],
                               bandEnergySide[],
                               bandOffset[],
                               mdctSpectrumLeft[],
                               mdctSpectrumRight[]
               */
  LOOP(1);
  for(i=0; i<numBands; i++) {

    MOVE(2);
    bandEnergyMid[i] = 0.0f;
    bandEnergySide[i] = 0.0f;

    LOOP(1);
    while (j < bandOffset[i+1]) {
        float specm, specs;

        ADD(2); MULT(2);
        specm = 0.5f * (mdctSpectrumLeft[j] + mdctSpectrumRight[j]);
        specs = 0.5f * (mdctSpectrumLeft[j] - mdctSpectrumRight[j]);

        MAC(2); STORE(2);
        bandEnergyMid[i]  += specm * specm;
        bandEnergySide[i] += specs * specs;

        j++;
    }
    ADD(2); STORE(2);
    *bandEnergyMidSum += bandEnergyMid[i];
		*bandEnergySideSum += bandEnergySide[i];

  }

  FLC_sub_end();
}

