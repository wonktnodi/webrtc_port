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

 $Id: env_extr.h,v 1.8.6.6 2004/04/01 07:31:14 ehr Exp $

*******************************************************************************/
/*
  \file
  \brief  Envelope extraction prototypes $Revision: 1.8.6.6 $
  \author Kristofer Kjoerling & Per Ekstrand
*/

#ifndef __ENVELOPE_EXTRACTION_H
#define __ENVELOPE_EXTRACTION_H

#include "sbr_const.h"
#include "sbr_bitb.h"
#include "sbrdecoder.h"
#include "sbrdecsettings.h"
#include "lpp_tran.h"
#include "string"

typedef enum
{
  HEADER_OK,
  HEADER_RESET,
  CONCEALMENT,
  HEADER_NOT_INITIALIZED
}
SBR_HEADER_STATUS;

typedef enum
{
  SBR_NOT_INITIALIZED,
  UPSAMPLING,
  SBR_ACTIVE
}
SBR_SYNC_STATE;


typedef enum
{
  COUPLING_OFF,
  COUPLING_LEVEL,
  COUPLING_BAL
}
COUPLING_MODE;

typedef enum
{
  UNDEFINED_CHANNEL_MODE,
  MONO,
  STEREO,
  PS_STEREO
}
CHANNEL_MODE;

struct PS_DEC;

typedef struct
{
  unsigned char nSfb[2];
  unsigned char nNfb;
  unsigned char numMaster;
  unsigned char lowSubband;
  unsigned char highSubband;
  unsigned char limiterBandTable[MAX_NUM_LIMITERS+1];
  unsigned char noLimiterBands;
  unsigned char nInvfBands;
  unsigned char *freqBandTable[2];
  unsigned char freqBandTableLo[MAX_FREQ_COEFFS/2+1];

  unsigned char freqBandTableHi[MAX_FREQ_COEFFS+1];

  unsigned char freqBandTableNoise[MAX_NOISE_COEFFS+1];

  unsigned char v_k_master[MAX_FREQ_COEFFS+1];

}
FREQ_BAND_DATA;

typedef FREQ_BAND_DATA *HANDLE_FREQ_BAND_DATA;

typedef struct _tagSBR_HEADER_DATA_DEC
{
  SBR_SYNC_STATE syncState;
  unsigned char frameErrorFlag;
  unsigned char prevFrameErrorFlag;
  unsigned char numberTimeSlots;
  unsigned char timeStep;
  unsigned short codecFrameSize;
  unsigned short outSampleRate;


  CHANNEL_MODE  channelMode;
  unsigned char ampResolution;


  unsigned char startFreq;
  unsigned char stopFreq;
  unsigned char xover_band;
  unsigned char freqScale;
  unsigned char alterScale;
  unsigned char noise_bands;


  unsigned char limiterBands;
  unsigned char limiterGains;
  unsigned char interpolFreq;
  unsigned char smoothingLength;

  FREQ_BAND_DATA FreqBandData;
  float sbr_GainSmooth[MAXNRSBRCHANNELS][MAX_FREQ_COEFFS];
  float sbr_NoiseSmooth[MAXNRSBRCHANNELS][MAX_FREQ_COEFFS];

  float sbr_OverlapBuffer[MAXNRQMFCHANNELS][2 * MAX_OV_COLS * NO_SYNTHESIS_CHANNELS];
  float sbr_CodecQmfStatesAnalysis[MAXNRQMFCHANNELS * QMF_FILTER_STATE_ANA_SIZE];
  _tagSBR_HEADER_DATA_DEC(){
	 reset();
	 memset(sbr_OverlapBuffer, 0, sizeof(sbr_OverlapBuffer));
	 memset(sbr_GainSmooth, 0, sizeof(sbr_GainSmooth));
	 memset(sbr_NoiseSmooth, 0, sizeof(sbr_GainSmooth));
	// memset(sbr_CodecQmfStatesAnalysis, 0, sizeof(sbr_CodecQmfStatesAnalysis));
  }

  void reset()
  {
	  syncState = SBR_NOT_INITIALIZED;
	  frameErrorFlag = 0;
	  prevFrameErrorFlag = 0;
	  numberTimeSlots = 16;
	  timeStep = 2;
	  codecFrameSize = 1024;
	  outSampleRate = 44100;

	  channelMode = UNDEFINED_CHANNEL_MODE;
	  ampResolution = SBR_AMP_RES_3_0;
	  startFreq = 5;
	  stopFreq = 0;
	  xover_band = 0;
	  freqScale = SBR_FREQ_SCALE_DEFAULT;
	  alterScale = SBR_ALTER_SCALE_DEFAULT;
	  noise_bands =  SBR_NOISE_BANDS_DEFAULT;
	  limiterBands = SBR_LIMITER_BANDS_DEFAULT;
	  limiterGains = SBR_LIMITER_GAINS_DEFAULT;
	  interpolFreq = SBR_INTERPOL_FREQ_DEFAULT;
	  smoothingLength = SBR_SMOOTHING_LENGTH_DEFAULT;

	  memset(&FreqBandData, 0, sizeof(FreqBandData));
  }
}
SBR_HEADER_DATA_DEC;

typedef SBR_HEADER_DATA_DEC *HANDLE_SBR_HEADER_DATA_DEC;


typedef struct
{
  unsigned char frameClass;
  unsigned char nEnvelopes;
  unsigned char borders[MAX_ENVELOPES+1];
  unsigned char freqRes[MAX_ENVELOPES];
  char          tranEnv;
  unsigned char nNoiseEnvelopes;
  unsigned char bordersNoise[MAX_NOISE_ENVELOPES+1];


}
FRAME_INFO;


typedef struct
{
  float sfb_nrg_prev[MAX_FREQ_COEFFS];
  float prevNoiseLevel[MAX_NOISE_COEFFS];
  unsigned char ampRes;
  unsigned char stopPos;
  char xposCtrl;
  COUPLING_MODE coupling;
  INVF_MODE sbr_invf_mode[MAX_INVF_BANDS];
}
SBR_PREV_FRAME_DATA;

typedef SBR_PREV_FRAME_DATA *HANDLE_SBR_PREV_FRAME_DATA;


typedef struct
{
  int nScaleFactors;

  FRAME_INFO frameInfo;
  unsigned char domain_vec[MAX_ENVELOPES];
  unsigned char domain_vec_noise[MAX_NOISE_ENVELOPES];

  INVF_MODE sbr_invf_mode[MAX_INVF_BANDS];
  COUPLING_MODE coupling;
  int ampResolutionCurrentFrame;

  char addHarmonics[MAX_FREQ_COEFFS];
  char xposCtrl;

  float  iEnvelope[MAX_NUM_ENVELOPE_VALUES];
  float  sbrNoiseFloorLevel[MAX_NUM_NOISE_VALUES];
}
SBR_FRAME_DATA;

typedef SBR_FRAME_DATA *HANDLE_SBR_FRAME_DATA;


void initSbrPrevFrameData (HANDLE_SBR_PREV_FRAME_DATA h_prev_data,
                           int timeSlots);

int sbrGetSingleChannelElement (HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
                                HANDLE_SBR_FRAME_DATA  hFrameData,
                                struct PS_DEC      *hPs,
                                HANDLE_BIT_BUFFER hBitBuf);

int sbrGetChannelPairElement (HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
                              HANDLE_SBR_FRAME_DATA hFrameDataLeft,
                              HANDLE_SBR_FRAME_DATA hFrameDataRight,
                              HANDLE_BIT_BUFFER hBitBuf);

SBR_HEADER_STATUS
sbrGetHeaderData (SBR_HEADER_DATA_DEC *h_sbr_header,
                  HANDLE_BIT_BUFFER hBitBuf,
                  SBR_ELEMENT_ID id_sbr);

void
initHeaderData  (SBR_HEADER_DATA_DEC *headerData,
                 int sampleRate,
                 int samplesPerFrame);
#endif
