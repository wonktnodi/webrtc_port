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

 $Id: sbr.h,v 1.2.2.5 2004/02/17 18:37:57 fh Exp $

*******************************************************************************/
/*!
  \file
  \brief  Main SBR structs definitions $Revision: 1.2.2.5 $
  \author Per Ekstrand & Kristofer Kjoerling
*/

#ifndef __SBR_H
#define __SBR_H

#include "qmf_enc.h"
#include "tran_det.h"
#include "fram_gen.h"
#include "nf_est.h"
#include "mh_det.h"
#include "invf_est.h"
#include "env_est.h"
#include "code_env.h"
#include "sbr_main.h"
#include "ton_corr.h"

struct SBR_BITSTREAM_DATA
{
  int TotalBits;
  int PayloadBits;
  int FillBits;
  int HeaderActive;
  int CRCActive;
  int NrSendHeaderData;
  int CountSendHeaderData;
  
  SBR_BITSTREAM_DATA(){
	  TotalBits = 0;
	  PayloadBits = 0;
	  FillBits = 0;
	  HeaderActive = 0;
	  CRCActive = 0;
	  NrSendHeaderData = 0;
	  CountSendHeaderData = 0;
  }
};

typedef struct SBR_BITSTREAM_DATA  *HANDLE_SBR_BITSTREAM_DATA;

struct SBR_HEADER_DATA_ENC
{

  int protocol_version;
  AMP_RES sbr_amp_res;
  int sbr_start_frequency;
  int sbr_stop_frequency;
  int sbr_xover_band;
  int sbr_noise_bands;
  int sbr_data_extra;
  int header_extra_1;
  int header_extra_2;
  int sbr_limiter_bands;
  int sbr_limiter_gains;
  int sbr_interpol_freq;
  int sbr_smoothing_length;
  int alterScale;
  int freqScale;

  SR_MODE sampleRateMode;

  int coupling;
  int prev_coupling;

  SBR_HEADER_DATA_ENC()
  {
	  memset(this, 0, sizeof(SBR_HEADER_DATA_ENC));
  }
};

typedef struct SBR_HEADER_DATA_ENC *HANDLE_SBR_HEADER_DATA_ENC;

struct SBR_CONFIG_DATA
{
  int nChannels;

  int nSfb[2];
  int num_Master;
  int sampleFreq;

  int xOverFreq;

  unsigned char  sbr_freqBandTableLO[MAX_FREQ_COEFFS/2+1];

  unsigned char    sbr_freqBandTableHI[MAX_FREQ_COEFFS+1];
  unsigned char *freqBandTable[2];

  unsigned char    sbr_v_k_master[MAX_FREQ_COEFFS +1];
  unsigned char *v_k_master;


  SBR_STEREO_MODE stereoMode;

  int detectMissingHarmonics;
  int useParametricCoding;
  int xposCtrlSwitch;
  SBR_CONFIG_DATA()
  {
	 nChannels = 0;
	 memset(nSfb, 0, sizeof(nSfb));
	 num_Master = 0;
	 sampleFreq = 0;
	 xOverFreq = 0;
	 memset(sbr_freqBandTableLO, 0, sizeof(sbr_freqBandTableLO));
	 memset(sbr_freqBandTableHI, 0, sizeof(sbr_freqBandTableHI));
	 memset(sbr_v_k_master, 0, sizeof(sbr_v_k_master));
	 detectMissingHarmonics = 0;
	 useParametricCoding = 0;
	 xposCtrlSwitch = 0;
  }
};

typedef struct SBR_CONFIG_DATA *HANDLE_SBR_CONFIG_DATA;

struct SBR_ENV_DATA
{
  int sbr_xpos_ctrl;
  FREQ_RES freq_res_fixfix;


  INVF_MODE sbr_invf_mode;
  INVF_MODE sbr_invf_mode_vec[MAX_NUM_NOISE_VALUES];

  XPOS_MODE sbr_xpos_mode;

  int ienvelope[MAX_ENVELOPES][MAX_FREQ_COEFFS];

  int codeBookScfLavBalance;
  int codeBookScfLav;
  const int *hufftableTimeC;
  const int *hufftableFreqC;
  const unsigned char *hufftableTimeL;
  const unsigned char *hufftableFreqL;

  const int *hufftableLevelTimeC;
  const int *hufftableBalanceTimeC;
  const int *hufftableLevelFreqC;
  const int *hufftableBalanceFreqC;
  const unsigned char *hufftableLevelTimeL;
  const unsigned char *hufftableBalanceTimeL;
  const unsigned char *hufftableLevelFreqL;
  const unsigned char *hufftableBalanceFreqL;


  const unsigned char *hufftableNoiseTimeL;
  const int *hufftableNoiseTimeC;
  const unsigned char *hufftableNoiseFreqL;
  const int *hufftableNoiseFreqC;

  const unsigned char *hufftableNoiseLevelTimeL;
  const int *hufftableNoiseLevelTimeC;
  const unsigned char *hufftableNoiseBalanceTimeL;
  const int *hufftableNoiseBalanceTimeC;
  const unsigned char *hufftableNoiseLevelFreqL;
  const int *hufftableNoiseLevelFreqC;
  const unsigned char *hufftableNoiseBalanceFreqL;
  const int *hufftableNoiseBalanceFreqC;

  HANDLE_SBR_GRID hSbrBSGrid;


  int syntheticCoding;
  int noHarmonics;
  int addHarmonicFlag;
  unsigned char addHarmonic[MAX_FREQ_COEFFS];

  int si_sbr_start_env_bits_balance;
  int si_sbr_start_env_bits;
  int si_sbr_start_noise_bits_balance;
  int si_sbr_start_noise_bits;

  int noOfEnvelopes;
  int noScfBands[MAX_ENVELOPES];
  int domain_vec[MAX_ENVELOPES];
  int domain_vec_noise[MAX_ENVELOPES];
  int sbr_noise_levels[MAX_NUM_NOISE_VALUES];
  int noOfnoisebands;

  int balance;
  int init_sbr_amp_res;

  SBR_ENV_DATA()
  {
	  memset(this, 0, sizeof(SBR_ENV_DATA));
  }
};


typedef struct SBR_ENV_DATA *HANDLE_SBR_ENV_DATA;

struct ENV_CHANNEL
{

  SBR_TRANSIENT_DETECTOR sbrTransientDetector;
  SBR_CODE_ENVELOPE sbrCodeEnvelope;
  SBR_CODE_ENVELOPE sbrCodeNoiseFloor;
  SBR_EXTRACT_ENVELOPE sbrExtractEnvelope;
  SBR_QMF_FILTER_BANK_ENC sbrQmf;
  SBR_ENVELOPE_FRAME SbrEnvFrame;
  SBR_TON_CORR_EST   TonCorr;

  struct SBR_ENV_DATA encEnvData;
};

typedef struct ENV_CHANNEL *HANDLE_ENV_CHANNEL;


#endif /* __SBR_H */
