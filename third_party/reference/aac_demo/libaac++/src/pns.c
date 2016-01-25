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

 $Id: pns.c,v 1.1.2.6 2004/02/16 17:53:57 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  perceptual noise substitution tool  $Revision: 1.1.2.6 $
  \author zerok   
*/

#include <math.h>
#include <assert.h>
#include "aac_rom_dec.h"
#include "channelinfo.h"
#include "block.h"
#include "pns.h"
#include "FFR_bitbuffer.h"

#include "flc.h" /* the 3GPP instrumenting tool */

extern const float sbr_randomPhase[AAC_NF_NO_RANDOM_VAL][2];

#define PNS_BAND_FLAGS_MASK              (PNS_BAND_FLAGS_SIZE - 1)
#define PNS_BAND_FLAGS_SHIFT             3

#define PNS_RANDOM_PHASE_16_32_BIT_SCALE 4
#define NOISE_OFFSET 90           /* cf. ISO 14496-3 p. 175 */



void GenerateRandomVector (AACDECODER self,
						   float scale,
                           float spec[],
                           int bandOffsetStart,
                           int bandOffsetEnd);

/*
  The function initializes the InterChannel data
*/
void CPns_InitInterChannelData(CAacDecoderChannelInfo *pAacDecoderChannelInfo) 
{
  unsigned int i;
  CPnsInterChannelData *pInterChannelData = &pAacDecoderChannelInfo->PnsInterChannelData;

  FLC_sub_start("CPns_InitInterChannelData");

  PTR_INIT(1); MOVE(1); /* counting previous operations */

  PTR_INIT(1); /* pInterChannelData->correlated[i] */
  LOOP(1);
  for (i = 0; i < PNS_BAND_FLAGS_SIZE; i++)
  {
    MOVE(1);
    pInterChannelData->correlated[i] = 0;
  }

  FLC_sub_end();
}


/*
  The function initializes the PNS data
*/
void CPns_InitPns(CAacDecoderChannelInfo *pAacDecoderChannelInfo) 
{
  unsigned int i;
  CPnsData *pPnsData = &pAacDecoderChannelInfo->PnsData;

  FLC_sub_start("CPns_InitPns");

  PTR_INIT(1); MOVE(1); /* counting previous operations */

  PTR_INIT(1); /* pPnsData->pnsUsed[i] */
  LOOP(1);
  for (i = 0; i < PNS_BAND_FLAGS_SIZE; i++)
  {
    MOVE(1);
    pPnsData->pnsUsed[i] = 0;
  }

  MOVE(2);
  pPnsData->PnsActive = 0;
  pPnsData->CurrentEnergy = 0;

  FLC_sub_end();
}


/*
  The function returns a value indicating whether PNS is used or not 
  acordding to the noise energy

  return:  PNS used
*/
int CPns_IsPnsUsed (CAacDecoderChannelInfo *pAacDecoderChannelInfo,
               const int group,
               const int band)
{
  CPnsData *pPnsData = &pAacDecoderChannelInfo->PnsData;
  unsigned pns_band = group*MaximumScaleFactorBandsShort+band;

  FLC_sub_start("CPns_IsPnsUsed");
  INDIRECT(1); PTR_INIT(1); MULT(1); ADD(1); /* counting previous operations */
  SHIFT(2); LOGIC(2); INDIRECT(1); /* counting post-operation */
  FLC_sub_end();

  return (pPnsData->pnsUsed[pns_band >> PNS_BAND_FLAGS_SHIFT] >> (pns_band & PNS_BAND_FLAGS_MASK)) & (unsigned char)1;
}

/*
  The function activates the noise correlation between the channel pair
*/
void CPns_SetCorrelation(CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                         const int group,
                         const int band)
{
  CPnsInterChannelData *pInterChannelData = &pAacDecoderChannelInfo->PnsInterChannelData;
  unsigned pns_band = group*MaximumScaleFactorBandsShort+band;

  FLC_sub_start("CPns_SetCorrelation");

  INDIRECT(1); PTR_INIT(1); MULT(1); ADD(1); /* counting previous operations */

  SHIFT(2); LOGIC(2); INDIRECT(1); STORE(1);
  pInterChannelData->correlated[pns_band >> PNS_BAND_FLAGS_SHIFT] |= (unsigned char)1 << (pns_band & PNS_BAND_FLAGS_MASK);

  FLC_sub_end();
}

/*
  The function indicates if the noise correlation between the channel pair
  is activated

  return:  PNS used
*/
int CPns_IsCorrelated(CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                      const int group,
                      const int band)
{
  CPnsInterChannelData *pInterChannelData = &pAacDecoderChannelInfo->PnsInterChannelData;
  unsigned pns_band = group*MaximumScaleFactorBandsShort+band;

  FLC_sub_start("CPns_IsCorrelated");
  INDIRECT(1); PTR_INIT(1); MULT(1); ADD(1); /* counting previous operations */
  SHIFT(2); LOGIC(2); INDIRECT(1); /* counting post-operation */
  FLC_sub_end();

  return (pInterChannelData->correlated[pns_band >> PNS_BAND_FLAGS_SHIFT] >> (pns_band & PNS_BAND_FLAGS_MASK)) & (unsigned char)1;
}

/*
  The function reads the PNS information from the bitstream
*/
void CPns_Read (CAacDecoderChannelInfo *pAacDecoderChannelInfo,
                HANDLE_BIT_BUF bs,
                const CodeBookDescription *hcb,
                unsigned char global_gain,
                int band,
                int group /* = 0 */)
{
  int delta ;
  unsigned pns_band = group*MaximumScaleFactorBandsShort+band;
  CPnsData *pPnsData = &pAacDecoderChannelInfo->PnsData;

  FLC_sub_start("CPns_Read");

  INDIRECT(1); PTR_INIT(1); MULT(1); ADD(1); /* counting previous operations */

  INDIRECT(1); BRANCH(1);
  if (pPnsData->PnsActive) {

    INDIRECT(1); FUNC(2); ADD(1);
    delta = CBlock_DecodeHuffmanWord (bs, hcb->CodeBook) - 60;
  } else {
    int noiseStartValue = GetBits(bs,9);

    FUNC(2); /* counting previous operation */

    ADD(1);
    delta = noiseStartValue - 256 ;

    MOVE(1);
    pPnsData->PnsActive = 1;

    INDIRECT(1); ADD(1); STORE(1);
    pPnsData->CurrentEnergy = global_gain - NOISE_OFFSET;
  }

  INDIRECT(1); ADD(1); STORE(1);
  pPnsData->CurrentEnergy += delta ;

  INDIRECT(1); MOVE(1);
  pAacDecoderChannelInfo->pScaleFactor[pns_band] = pPnsData->CurrentEnergy;

  SHIFT(2); LOGIC(2); INDIRECT(1); STORE(1);
  pPnsData->pnsUsed[pns_band >> PNS_BAND_FLAGS_SHIFT] |= (unsigned char)1 << (pns_band & PNS_BAND_FLAGS_MASK);

  FLC_sub_end();
}

#include "aacdecoder_def.h"
/*
  The function applies PNS (i.e. it generates noise) on the bands
  flagged as noisy bands
*/
void CPns_Apply (AACDECODER self, CAacDecoderChannelInfo *pAacDecoderChannelInfo[],
                 int channel)
{
  int window, group, groupwin, band;
  CPnsData *pPnsData = &pAacDecoderChannelInfo[channel]->PnsData;
  CIcsInfo *pIcsInfo = &pAacDecoderChannelInfo[channel]->IcsInfo;

  FLC_sub_start("CPns_Apply");

  INDIRECT(2); PTR_INIT(2); /* counting previous operation */

  BRANCH(1);
  if (channel == 0) {

    MOVE(2);
    self->noise_left_index = self->noise_left_index_start = self->noise_index;
  } else {

    MOVE(1);
    self->noise_left_index = self->noise_left_index_start;
  }

  INDIRECT(1); BRANCH(1);
  if (pPnsData->PnsActive) {
    const short *BandOffsets = GetScaleFactorBandOffsets(pIcsInfo);
    int fft_exp = IsLongBlock(&pAacDecoderChannelInfo[channel]->IcsInfo)? 9 : 6; /* This coefficient is related to the spectral exponent used in the
                                                                                    requantization (see functions CLongBlock_ReadSpectralData() and 
                                                                                    CShortBlock_ReadSpectralData() ).
                                                                                 */

    FUNC(1); FUNC(1); PTR_INIT(1); MOVE(1); /* counting previous operations */

    FUNC(1); LOOP(1);
    for (window = 0, group = 0; group < GetWindowGroups(pIcsInfo); group++) {

      FUNC(1); LOOP(1);
      for (groupwin = 0; groupwin < GetWindowGroupLength(pIcsInfo, group); groupwin++, window++) {
        float *spectrum = &pAacDecoderChannelInfo[channel]->pSpectralCoefficient[FRAME_SIZE/8*window];

        INDIRECT(1); PTR_INIT(1); /* counting previous operation */

        PTR_INIT(1); /* BandOffsets[band] */
        FUNC(1); LOOP(1);
        for (band = 0 ; band < GetScaleFactorBandsTransmitted(pIcsInfo); band++) {

          FUNC(3); BRANCH(1);
          if (CPns_IsPnsUsed (pAacDecoderChannelInfo[channel], group, band)) {
            unsigned pns_band = group*MaximumScaleFactorBandsShort+band;
            float scale = (float) pow(2.0, 0.25 * pAacDecoderChannelInfo[channel]->pScaleFactor[pns_band] - fft_exp);

            MULT(2); ADD(2); TRANS(1); /* counting previous operation */

            FUNC(3); LOGIC(1); BRANCH(1);
            if (channel > 0 && CPns_IsCorrelated(pAacDecoderChannelInfo[0], group, band)) {
              int noise_index_tmp = self->noise_index;
              self->noise_index = self->noise_left_index;

              MOVE(2); /* counting previous operation */

              FUNC(4);
              GenerateRandomVector(self,
								   scale,
                                   spectrum,
                                   BandOffsets[band],
                                   BandOffsets[band + 1]);

              MOVE(1);
              self->noise_index = noise_index_tmp;
            } else {

              FUNC(4);
              GenerateRandomVector(self,
								   scale,
                                   spectrum,
                                   BandOffsets[band],
                                   BandOffsets[band + 1]);
            }
          }
        }
      }
    }
  } 

  FLC_sub_end();
}

void GenerateRandomVector (AACDECODER self,
						   float scale,
                           float spec[],
                           int bandOffsetStart,
                           int bandOffsetEnd)
{
  int i;
  float nrg = 0.0f;

  FLC_sub_start("GenerateRandomVector");

  MOVE(1); /* counting previous operation */

  PTR_INIT(2); /* spec[i]
                  sbr_randomPhase[noise_index][0]
               */
  LOOP(1);
  for (i = bandOffsetStart; i < bandOffsetEnd; i++) {

    MOVE(1);
    spec[i] = sbr_randomPhase[self->noise_index][0];

    MAC(1);
    nrg += spec[i] * spec[i];

    self->noise_index = (self->noise_index + 1) & (AAC_NF_NO_RANDOM_VAL - 1);
    self->noise_left_index = (self->noise_left_index + 1) & (AAC_NF_NO_RANDOM_VAL - 1);
  }

  TRANS(1); DIV(1);
  scale /= (float) sqrt(nrg);

  PTR_INIT(1); /* spec[i] */
  LOOP(1);
  for (i = bandOffsetStart; i < bandOffsetEnd; i++) {

    MULT(1); STORE(1);
    spec[i] *= scale;
  }

  FLC_sub_end();
}
