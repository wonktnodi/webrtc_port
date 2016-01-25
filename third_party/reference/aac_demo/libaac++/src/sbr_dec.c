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

 $Id: sbr_dec.c,v 1.1.2.12 2004/05/31 12:53:11 ehr Exp $

*******************************************************************************/
/*
  \file
  \brief  Sbr decoder $Revision: 1.1.2.12 $
  This module provides the actual decoder implementation
*/
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "sbr_ram_dec.h"
#include "sbr_dec.h"
#include "sbrdecsettings.h"
#include "env_extr.h"
#include "env_calc.h"
#include "FloatFR.h"
#include "ps_dec.h"

#include "flc.h" /* the 3GPP instrumenting tool */

/*
  \brief      SBR decoder core function for one channel
*/


void
sbr_dec (HANDLE_SBR_DEC hSbrDec,
         float *timeIn,
         float *timeOut,
         float *interimResult,
         HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
         HANDLE_SBR_FRAME_DATA hFrameData,
         HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData,
         int applyProcessing
         ,HANDLE_PS_DEC h_ps_d,
         HANDLE_SBR_QMF_FILTER_BANK_DEC hSynthesisQmfBankRight
#ifdef NON_BE_BUGFIX
         , int nChannels
#endif
         )
{
  int i, k, slot;
  int ov_len;
  int bUseLP=1;

  float  *QmfBufferReal[MAX_ENV_COLS];
#ifndef LP_SBR_ONLY
  float  *QmfBufferImag[MAX_ENV_COLS];
#endif
  float  *ptr;

  int noCols = hHeaderData->numberTimeSlots * hHeaderData->timeStep;

  int halflen = noCols >> 1;
  int islots = noCols >> 2;

  FLC_sub_start("sbr_dec");

  INDIRECT(2); MULT(1); SHIFT(2); /* counting previous operations */

#ifdef NON_BE_BUGFIX
  BRANCH(1);
  if (nChannels == 1)
#else
  INDIRECT(1); ADD(1); BRANCH(1);
  if(hHeaderData->channelMode==PS_STEREO)
#endif
  {
    MOVE(1);
    bUseLP = 0;
  }


#ifndef LP_SBR_ONLY
  INDIRECT(1); ADD(1); BRANCH(1);
  if(hHeaderData->channelMode==MONO) {
    MOVE(1);
    bUseLP = 0;
  }
#endif


  MOVE(1);
  ov_len = 6;

  BRANCH(1);
  if (bUseLP) {

    INDIRECT(1); PTR_INIT(1);
    ptr = hSbrDec->ptrsbr_OverlapBuffer;

    PTR_INIT(1); /* QmfBufferReal[slot] */
    LOOP(1);
    for(slot=0; slot<ov_len; slot++) {
      PTR_INIT(1);
      QmfBufferReal[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;

    }

    MULT(1); ADD(1); PTR_INIT(1);
    ptr = timeOut + islots * 2 * NO_SYNTHESIS_CHANNELS;

    PTR_INIT(1); /* QmfBufferReal[slot] */
    ADD(1); MULT(1); LOOP(1);
    for(i=0; i<2*(halflen-islots); i++) {
      PTR_INIT(1);
      QmfBufferReal[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }

    MOVE(1);
    ptr = timeIn;

    PTR_INIT(1); /* QmfBufferReal[slot] */
    MULT(1); LOOP(1);
    for(i=0; i<2*islots; i++) {
      PTR_INIT(1);
      QmfBufferReal[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }

    assert( sizeof(SBR_FRAME_DATA) <= islots * 2 * NO_SYNTHESIS_CHANNELS * sizeof(float) );
  }

#ifndef LP_SBR_ONLY
  else {

    PTR_INIT(1);
    ptr = hSbrDec->ptrsbr_OverlapBuffer;

    PTR_INIT(2); /* QmfBufferReal[slot]
                    QmfBufferImag[slot]
                 */
    LOOP(1);
    for(slot=0; slot<ov_len; slot++) {

      PTR_INIT(1);
      QmfBufferReal[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;

      PTR_INIT(1);
      QmfBufferImag[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;
    }

    MULT(1); ADD(1); PTR_INIT(1);
    ptr = timeOut + islots * 2 * NO_SYNTHESIS_CHANNELS;

    PTR_INIT(2); /* QmfBufferReal[slot]
                    QmfBufferImag[slot]
                 */
    ADD(1); LOOP(1);
    for(i=0; i<halflen-islots; i++) {

      PTR_INIT(1);
      QmfBufferReal[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;

      PTR_INIT(1);
      QmfBufferImag[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }

    PTR_INIT(1);
    ptr = &hSbrDec->workBuffer2[0];

    PTR_INIT(2); /* QmfBufferReal[slot]
                    QmfBufferImag[slot]
                 */
    LOOP(1);
    for(i=0; i<halflen; i++) {

      PTR_INIT(1);
      QmfBufferReal[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;

      PTR_INIT(1);
      QmfBufferImag[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }

    INDIRECT(1); ADD(1); BRANCH(1);
    if(hHeaderData->channelMode==PS_STEREO)
    {
      MOVE(1);
      ptr=interimResult;
    }
    else
    {
      MOVE(1);
      ptr = timeIn;
    }

    PTR_INIT(2); /* QmfBufferReal[slot]
                    QmfBufferImag[slot]
                 */
    LOOP(1);
    for(i=0; i<islots; i++) {

      PTR_INIT(1);
      QmfBufferReal[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;

      PTR_INIT(1);
      QmfBufferImag[slot] = ptr;

      ptr += NO_SYNTHESIS_CHANNELS;
      slot++;
    }

    assert( sizeof(SBR_FRAME_DATA) <= islots * 2 * NO_SYNTHESIS_CHANNELS * sizeof(float) );
  }
#endif /* #ifndef LP_SBR_ONLY */



  assert(slot == noCols+ov_len);



#ifndef LP_SBR_ONLY
  FUNC(5); PTR_INIT(2);
#else
  FUNC(4); PTR_INIT(2);
#endif
  cplxAnalysisQmfFiltering (timeIn,
                            QmfBufferReal + ov_len,
#ifndef LP_SBR_ONLY
                            QmfBufferImag + ov_len,
#endif
                            &hSbrDec->CodecQmfBank,
                            bUseLP);


#ifndef LP_SBR_ONLY
  PTR_INIT(2); /* QmfBufferReal[]
                  QmfBufferImag[]
               */
#else
  PTR_INIT(1); /* QmfBufferReal[] */
#endif
  ADD(1); LOOP(1);
  for (slot = ov_len; slot < noCols+ov_len; slot++) {

    LOOP(1);
    for (k=NO_ANALYSIS_CHANNELS; k<NO_SYNTHESIS_CHANNELS; k++){

      MOVE(1);
      QmfBufferReal[slot][k] = 0;
#ifndef LP_SBR_ONLY
      BRANCH(1);
      if (!bUseLP)
      {
        MOVE(1);
        QmfBufferImag[slot][k] = 0;
      }
#endif
    }
  }

  BRANCH(1);
  if (applyProcessing)
  {
    float degreeAlias[NO_SYNTHESIS_CHANNELS]
#if defined NON_BE_BUGFIX
      = {0.0f}
#endif
    ;

    unsigned char * borders = hFrameData->frameInfo.borders;

    INDIRECT(1); PTR_INIT(1); /* counting previous operation */

#ifndef LP_SBR_ONLY
    ADD(1); INDIRECT(7); PTR_INIT(1); FUNC(11);
#else
    ADD(1); INDIRECT(7); PTR_INIT(1); FUNC(10);
#endif
    lppTransposer ( &hSbrDec->LppTrans,
                    QmfBufferReal,
#ifndef LP_SBR_ONLY
                    QmfBufferImag,
#endif
                    degreeAlias,
                    hHeaderData->timeStep,
                    borders[0],
                    borders[hFrameData->frameInfo.nEnvelopes] - hHeaderData->numberTimeSlots,
                    hHeaderData->FreqBandData.nInvfBands,
                    hFrameData->sbr_invf_mode,
                    hPrevFrameData->sbr_invf_mode,
                    bUseLP
                    );

#ifndef LP_SBR_ONLY
    INDIRECT(1); PTR_INIT(1); FUNC(7);
#else
    INDIRECT(1); PTR_INIT(1); FUNC(6);
#endif
    calculateSbrEnvelope (&hSbrDec->SbrCalculateEnvelope,
                          hHeaderData,
                          hFrameData,
                          QmfBufferReal,
#ifndef LP_SBR_ONLY
                          QmfBufferImag,
#endif
                          degreeAlias,
                          bUseLP
                          );

    PTR_INIT(2); /* hPrevFrameData->sbr_invf_mode[]
                    hFrameData->sbr_invf_mode[]
                 */
    INDIRECT(1); LOOP(1);
    for (i=0; i<hHeaderData->FreqBandData.nInvfBands; i++) {
      MOVE(1);
      hPrevFrameData->sbr_invf_mode[i] = hFrameData->sbr_invf_mode[i];
    }

    INDIRECT(2); MOVE(1);
    hPrevFrameData->coupling = hFrameData->coupling;
    hPrevFrameData->xposCtrl = hFrameData->xposCtrl;

    INDIRECT(2); MOVE(2);
    hPrevFrameData->ampRes = hFrameData->ampResolutionCurrentFrame;
  }
  else {
    FUNC(2); LOOP(1); PTR_INIT(1); MOVE(1); STORE(LPC_ORDER * NO_ANALYSIS_CHANNELS);
    memset( hSbrDec->LppTrans.lpcFilterStatesReal[0], 0, LPC_ORDER * NO_ANALYSIS_CHANNELS * sizeof(float) );
#ifndef LP_SBR_ONLY
    BRANCH(1);
    if (!bUseLP)
    {
      FUNC(2); LOOP(1); PTR_INIT(1); MOVE(1); STORE(LPC_ORDER * NO_ANALYSIS_CHANNELS);
      memset( hSbrDec->LppTrans.lpcFilterStatesImag[0], 0, LPC_ORDER * NO_ANALYSIS_CHANNELS * sizeof(float) );
    }
#endif
  }



  INDIRECT(1); BRANCH(1);
  if(hSbrDec->bApplyQmfLp){

#ifndef LP_SBR_ONLY
    PTR_INIT(2); /* QmfBufferReal[]
                    QmfBufferImag[]
                 */
#else
    PTR_INIT(1); /* QmfBufferReal[] */
#endif
    LOOP(1);
    for (i = 0; i < noCols; i++){
      FUNC(2); LOOP(1); PTR_INIT(1); MOVE(1); STORE(NO_SYNTHESIS_CHANNELS-hSbrDec->qmfLpChannel);
      memset(QmfBufferReal[i] + hSbrDec->qmfLpChannel,0,(NO_SYNTHESIS_CHANNELS-hSbrDec->qmfLpChannel)*sizeof(float));
#ifndef LP_SBR_ONLY
    BRANCH(1);
    if (!bUseLP)
    {
      FUNC(2); LOOP(1); PTR_INIT(1); MOVE(1); STORE(NO_SYNTHESIS_CHANNELS-hSbrDec->qmfLpChannel);
      memset(QmfBufferImag[i] + hSbrDec->qmfLpChannel,0,(NO_SYNTHESIS_CHANNELS-hSbrDec->qmfLpChannel)*sizeof(float));
    }
#endif
    }
  }



#ifndef MONO_ONLY

  INDIRECT(1); ADD(1); BRANCH(1);
  if(hHeaderData->channelMode==PS_STEREO) {

    PTR_INIT(1); MULT(1); ADD(1); FUNC(7);
    cplxSynthesisQmfFiltering (QmfBufferReal,
                               QmfBufferImag,
                               timeOut-noCols*NO_SYNTHESIS_CHANNELS,
                               &hSbrDec->SynthesisQmfBank,
                               bUseLP,
                               h_ps_d,
                               1);

    FUNC(7);
    cplxSynthesisQmfFiltering (QmfBufferReal,
                               QmfBufferImag,
                               timeOut,
                               hSynthesisQmfBankRight,
                               bUseLP,
                               h_ps_d,
                               0);
  }
  else {

#endif /* #ifndef MONO_ONLY */


#ifndef LP_SBR_ONLY
    FUNC(7);
#else
    FUNC(6);
#endif
    cplxSynthesisQmfFiltering (QmfBufferReal,
#ifndef LP_SBR_ONLY
                               QmfBufferImag,
#endif
                               timeOut,
                               &hSbrDec->SynthesisQmfBank,
                               bUseLP,
                               h_ps_d,
                               0
                               );

#ifndef MONO_ONLY
  }
#endif /* #ifndef MONO_ONLY */


#ifndef LP_SBR_ONLY
    PTR_INIT(2); /* QmfBufferReal[]
                    QmfBufferImag[]
                 */
#else
    PTR_INIT(1); /* QmfBufferReal[] */
#endif

  LOOP(1);
  for ( i=0; i<ov_len; i++ ) {

    LOOP(1);
    for ( k=0; k<NO_SYNTHESIS_CHANNELS; k++ ) {

      MOVE(1);
      QmfBufferReal[i][k] = QmfBufferReal[i+noCols][k];
#ifndef LP_SBR_ONLY
      BRANCH(1);
      if (!bUseLP)
      {
        MOVE(1);
        QmfBufferImag[i][k] = QmfBufferImag[i+noCols][k];
      }
#endif
    }
  }

  FLC_sub_end();
}


/*
  \brief     Creates sbr decoder structure
  \return    errorCode
*/
int
createSbrDec (SBR_CHANNEL * hSbrChannel,
              HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
              int chan,
              int bApplyQmfLp,
              int sampleFreq
              )

{
  int err;
  int timeSlots = hHeaderData->numberTimeSlots;
  HANDLE_SBR_DEC hs = &(hSbrChannel->SbrDec);

  FLC_sub_start("createSbrDec");

  INDIRECT(2); MOVE(1); PTR_INIT(1); /* counting previous operation */

  INDIRECT(2); PTR_INIT(1);
  hSbrChannel->hPrevFrameData = &hSbrChannel->PrevFrameData[chan];


  INDIRECT(1); MOVE(1);
  hs->bApplyQmfLp  = bApplyQmfLp;

  ADD(1); BRANCH(1);
  if(bApplyQmfLp == 8)
  {
    DIV(1); MULT(1); INDIRECT(1); STORE(1);
    hs->qmfLpChannel = (unsigned char)(8000.0f/sampleFreq * NO_SYNTHESIS_CHANNELS);
  }

  ADD(1); BRANCH(1);
  if(bApplyQmfLp == 4)
  {
    DIV(1); MULT(1); INDIRECT(1); STORE(1);
    hs->qmfLpChannel = (unsigned char)(4000.0f/sampleFreq * NO_SYNTHESIS_CHANNELS);
  }


  INDIRECT(1); PTR_INIT(1); FUNC(3);
  err = createSbrEnvelopeCalc (&hs->SbrCalculateEnvelope,
                               hHeaderData,
                               chan);

  BRANCH(1);
  if (err) {
    FLC_sub_end();
    return (-1);
  }

  INDIRECT(1); FUNC(2);
  initSbrPrevFrameData (hSbrChannel->hPrevFrameData, timeSlots);

  FLC_sub_end();

  return 0;
}



/*
  \brief     Creates sbr decoder structure
  \return    errorCode
*/
int
createSbrQMF (SBR_CHANNEL * hSbrChannel,
              HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
              int chan,
              int bDownSample
              )

{
  int err;
  int timeSlots = hHeaderData->numberTimeSlots;
  int noCols = timeSlots * hHeaderData->timeStep;
  HANDLE_SBR_DEC hs = &(hSbrChannel->SbrDec);

  FLC_sub_start("createSbrQMF");

  INDIRECT(2); MOVE(1); MULT(1); PTR_INIT(1); /* counting previous operation */


  INDIRECT(3); PTR_INIT(1); FUNC(5);
  createCplxAnalysisQmfBank (&hs->CodecQmfBank,
							 hHeaderData,
                             noCols,
                             hHeaderData->FreqBandData.lowSubband,
                             hHeaderData->FreqBandData.highSubband,
                             chan);

  INDIRECT(1); PTR_INIT(1); FUNC(6);
  createCplxSynthesisQmfBank (&hs->SynthesisQmfBank,
                              noCols,
                              hHeaderData->FreqBandData.lowSubband,
                              hHeaderData->FreqBandData.highSubband,
                              chan,
                              bDownSample);


  INDIRECT(9); PTR_INIT(1); FUNC(10);
  err = createLppTransposer (&hs->LppTrans,
                             hHeaderData->FreqBandData.lowSubband,
                             hHeaderData->FreqBandData.v_k_master,
                             hHeaderData->FreqBandData.numMaster,
                             hs->SynthesisQmfBank.usb,
                             hs->CodecQmfBank.no_col,
                             hHeaderData->FreqBandData.freqBandTableNoise,
                             hHeaderData->FreqBandData.nNfb,
                             hHeaderData->outSampleRate,
                             chan);
  BRANCH(1);
  if (err) {
    FLC_sub_end();
    return (-1);
  }

  INDIRECT(1); PTR_INIT(1);
  hs->ptrsbr_OverlapBuffer = hHeaderData->sbr_OverlapBuffer[chan];

  FLC_sub_end();

  return 0;
}


/*
  \brief     resets sbr QMF structure
  \return    errorCode,
*/
int
resetSbrQMF (HANDLE_SBR_DEC hSbrDec,
             HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
             int sbrChannel,
#ifdef NON_BE_BUGFIX
             int nChannels,
#endif
             HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData)
{
  int old_lsb = hSbrDec->SynthesisQmfBank.lsb;
  int new_lsb = hHeaderData->FreqBandData.lowSubband;
  int k,l, startBand, stopBand, startSlot;
  float * ptr;
#ifdef NON_BE_BUGFIX
  int bUseLP=1;
#endif

  float  *OverlapBufferReal[6];
#ifndef LP_SBR_ONLY
  float  *OverlapBufferImag[6];
#endif

  FLC_sub_start("resetSbrQMF");

  INDIRECT(2); MOVE(2); /* counting previous operations */

#ifdef NON_BE_BUGFIX
  INDIRECT(1); ADD(2); LOGIC(1); BRANCH(1);
  if (nChannels == 1)
  {
    MOVE(1);
    bUseLP = 0;
  }
#endif

  INDIRECT(1); PTR_INIT(1);
  ptr = hSbrDec->ptrsbr_OverlapBuffer;

#ifndef LP_SBR_ONLY
  PTR_INIT(2); /* OverlapBufferReal[]
                  OverlapBufferImag[]
               */
#else
  PTR_INIT(1); /* OverlapBufferReal[] */
#endif
  LOOP(1);
  for(l=0; l<6; l++) {

    PTR_INIT(1);
    OverlapBufferReal[l] = ptr;

    ADD(1);
    ptr += NO_SYNTHESIS_CHANNELS;

#ifndef LP_SBR_ONLY
#ifdef NON_BE_BUGFIX
    BRANCH(1);
    if (!bUseLP) {
#endif
      PTR_INIT(1);
      OverlapBufferImag[l] = ptr;

      ADD(1);
      ptr += NO_SYNTHESIS_CHANNELS;
#ifdef NON_BE_BUGFIX
    }
#endif
#endif
  }


  INDIRECT(4); MOVE(2);
  hSbrDec->SynthesisQmfBank.lsb = hHeaderData->FreqBandData.lowSubband;
  hSbrDec->SynthesisQmfBank.usb = hHeaderData->FreqBandData.highSubband;

  INDIRECT(2); MOVE(2);
  hSbrDec->CodecQmfBank.lsb = hSbrDec->SynthesisQmfBank.lsb;
  hSbrDec->CodecQmfBank.usb = hSbrDec->SynthesisQmfBank.usb;


  MOVE(2);
  startBand = old_lsb;
  stopBand =  new_lsb;

  INDIRECT(3); ADD(1); MULT(1);
  startSlot = hHeaderData->timeStep * (hPrevFrameData->stopPos - hHeaderData->numberTimeSlots);

  LOOP(1);
  for (l=startSlot; l<6 ; l++) {

#ifndef LP_SBR_ONLY
    PTR_INIT(2); /* OverlapBufferReal[][]
                    OverlapBufferImag[][]
                 */
#else
    PTR_INIT(1); /* OverlapBufferReal[][] */
#endif
    LOOP(1);
    for (k=startBand; k<stopBand; k++) {
      MOVE(1);
      OverlapBufferReal[l][k] = 0;

#ifndef LP_SBR_ONLY
#ifdef NON_BE_BUGFIX
      BRANCH(1);
      if (!bUseLP) {
#endif
        MOVE(1);
        OverlapBufferImag[l][k] = 0;
#ifdef NON_BE_BUGFIX
      }
#endif
#endif
    }
  }


  BRANCH(1);
  if (sbrChannel == 0) {

    ADD(1); BRANCH(1); MOVE(1);
    startBand = min(old_lsb,new_lsb);
    ADD(1); BRANCH(1); MOVE(1);
    stopBand =  max(old_lsb,new_lsb);
    
#ifndef LP_SBR_ONLY
    PTR_INIT(4); /* hSbrDec->LppTrans.lpcFilterStatesReal[0][l]
                    hSbrDec->LppTrans.lpcFilterStatesReal[1][l]
                    hSbrDec->LppTrans.lpcFilterStatesImag[0][l]
                    hSbrDec->LppTrans.lpcFilterStatesImag[1][l]
                 */
#else
    PTR_INIT(2); /* hSbrDec->LppTrans.lpcFilterStatesReal[0][l]
                    hSbrDec->LppTrans.lpcFilterStatesReal[1][l]
                 */
#endif
    LOOP(1);
    for (l=startBand; l<stopBand; l++) {
      MOVE(1);
      hSbrDec->LppTrans.lpcFilterStatesReal[0][l] =  hSbrDec->LppTrans.lpcFilterStatesReal[1][l] = 0;
#ifndef LP_SBR_ONLY
     MOVE(1);
     hSbrDec->LppTrans.lpcFilterStatesImag[0][l] =  hSbrDec->LppTrans.lpcFilterStatesImag[1][l] = 0;
#endif
    }
  }

  FLC_sub_end();

  return 0;
}
