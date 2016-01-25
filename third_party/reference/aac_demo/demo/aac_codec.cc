#include "aac_codec.h"
#include <stdlib.h>
#include <memory>
#include <assert.h>

extern "C" {
#include "aacenc.h"
#include "sbr_main.h"
#include "resampler.h"
#include "iir32resample.h"

extern float sbr_envRBuffer[];
};

#define CORE_DELAY   (1600)
#define INPUT_DELAY  ((CORE_DELAY)*2 +6*64-2048+1)  /* ((1600 (core codec)*2 (multi rate) + 6*64 (sbr dec delay) - 2048 (sbr enc delay) + magic*/
#define MAX_DS_FILTER_DELAY 16  /* the additional max resampler filter delay (source fs)*/

#define CORE_INPUT_OFFSET_PS (0)  /* (96-64) makes AAC still some 64 core samples too early wrt SBR ... maybe -32 would be even more correct, but 1024-32 would need additional SBR bitstream delay by one frame */


struct aac_encoder_t {
  AACENC_CONFIG _config;
  HANDLE_SBR_ENCODER   _enc_env;
  struct AAC_ENCODER *_enc;
  unsigned int _aac_sample_rate;
  int use_parametric_stereo;
  int bDoIIR32Resample;
  float inputBuffer[(AACENC_BLOCKSIZE * 2 + MAX_DS_FILTER_DELAY + INPUT_DELAY)*MAX_CHANNELS];
  int bEncodeMono;
  int nChannelsAAC, nChannelsSBR;
  unsigned int sampleRateAAC;
  float *resamplerScratch;
  int nSamplesPerChannel;
  int writeOffset;
  int input_channls;
  int nDSOutBlockSize;
  int stride;
};

static IIR21_RESAMPLER IIR21_reSampler[MAX_CHANNELS];

int open_aac_encoder(struct aac_encoder_t *&enc, int sample_rate, int bitrate,
                     int chl_in, int chl_out, int bandwidth /*= 0*/) {
  int nChannelsAAC, nChannelsSBR;
  struct aac_encoder_t *coder =
    (struct aac_encoder_t*)malloc(sizeof(struct aac_encoder_t));
  memset(coder, 0, sizeof(struct aac_encoder_t));

  if (0 == coder) return -1;

  unsigned int numAncDataBytes = 0;
  unsigned int ancDataLength = 0;

  /*!< required only for interfacing with audio output library, thus not counted into RAM requirements */

  int bDoIIR2Downsample = 0;
  int bDingleRate = 0;
  int useParametricStereo = 0;
  int coreWriteOffset = 0;
  int coreReadOffset = 0;
  int envWriteOffset = 0;
  int envReadOffset = 0;
  coder->writeOffset = INPUT_DELAY * MAX_CHANNELS;

  int bDoUpsample = 0;
  int upsampleReadOffset = 0;

  int bDoIIR32Resample = 0;

  const int nRuns = 4;
  coder->resamplerScratch = sbr_envRBuffer;

  if ((!coder->bEncodeMono) && (2 != chl_in)) {
    fprintf(stderr, "Need stereo input for stereo coding mode !\n");
    goto l_error;
  }

#if (MAX_CHANNELS==1)

  if (!bEncodeMono) {
    fprintf(stderr, "Mono encoder cannot encode stereo coding mode !\n");
    exit(10);
  }

#endif

  /*
    set up basic parameters for aacPlus codec
  */
  AacInitDefaultConfig(&coder->_config);

  nChannelsAAC = nChannelsSBR = coder->bEncodeMono ? 1 : chl_in;

  if ((2 == chl_in) && (!coder->bEncodeMono) && (bitrate >= 16000)
      && (bitrate < 44001)) {
    useParametricStereo = 1;
  }

  if (useParametricStereo) {
    nChannelsAAC = 1;
    nChannelsSBR = 2;
  }

  if ( (48000 == sample_rate) && (2 == nChannelsAAC) && (bitrate < 24000) ||
       (48000 == sample_rate) && (1 == nChannelsAAC) && (bitrate < 12000)) {
    bDoIIR32Resample  = 1;
  }

  if (16000 == sample_rate) {
    bDoUpsample = 1;
    sample_rate = 32000;
    bDingleRate = 1;
  }

  coder->sampleRateAAC = sample_rate;

  if (bDoIIR32Resample) {
    coder->sampleRateAAC = 32000;
  }

  coder->_config.bitRate = bitrate;
  coder->_config.nChannelsIn = chl_in;
  coder->_config.nChannelsOut = nChannelsAAC;
  coder->_config.bandWidth = bandwidth;

  /*
    set up SBR configuration
  */
  if(!IsSbrSettingAvail (bitrate, nChannelsAAC, coder->sampleRateAAC,
                         &coder->sampleRateAAC)) {
    fprintf(stderr, "No valid SBR configuration found\n");
    goto l_error;
  }

  {
    sbrConfiguration sbrConfig;

    envReadOffset = 0;
    coreWriteOffset = 0;

    if(useParametricStereo) {
      envReadOffset = (MAX_DS_FILTER_DELAY + INPUT_DELAY) * MAX_CHANNELS;
      coreWriteOffset = CORE_INPUT_OFFSET_PS;
      coder->writeOffset = envReadOffset;
    }

    InitializeSbrDefaults (&sbrConfig);

    sbrConfig.usePs = useParametricStereo;

    AdjustSbrSettings(&sbrConfig,
                      bitrate,
                      nChannelsAAC,
                      coder->sampleRateAAC,
                      AACENC_TRANS_FAC,
                      24000);

    EnvOpen  (&coder->_enc_env,
              coder->inputBuffer + coreWriteOffset,
              &sbrConfig,
              &coder->_config.bandWidth);

    /* set IIR 2:1 downsampling */
    bDoIIR2Downsample = (bDoUpsample) ? 0 : 1;

    if (useParametricStereo) {
      bDoIIR2Downsample = 0;
    }
  }

  /*
    set up mode-dependant 1:2 upsampling
  */
  if (bDoUpsample) {
    if (chl_in > 1) {
      fprintf( stderr, "\n Stereo @ 16kHz input sample rate is not supported\n");
      goto l_error;
    }

    InitIIR21_Resampler(&(IIR21_reSampler[0]));

#if (MAX_CHANNELS==2)
    InitIIR21_Resampler(&(IIR21_reSampler[1]));
#endif

    assert(IIR21_reSampler[0].delay <= MAX_DS_FILTER_DELAY);

    if (useParametricStereo) {
      coder->writeOffset   += AACENC_BLOCKSIZE * MAX_CHANNELS;

      upsampleReadOffset  = coder->writeOffset;
      envWriteOffset  = envReadOffset;
    } else {
      coder->writeOffset += AACENC_BLOCKSIZE * MAX_CHANNELS;

      coreReadOffset = coder->writeOffset;

      upsampleReadOffset  = coder->writeOffset - (((INPUT_DELAY - IIR21_reSampler[0].delay)
                                            >> 1) * MAX_CHANNELS);

      envWriteOffset = ((INPUT_DELAY - IIR21_reSampler[0].delay) &  0x1) *
                       MAX_CHANNELS;
      envReadOffset = 0;
    }
  } else {

    /*
      set up 2:1 downsampling
    */
    if (bDoIIR2Downsample) {
      InitIIR21_Resampler(&(IIR21_reSampler[0]));

#if (MAX_CHANNELS==2)
      InitIIR21_Resampler(&(IIR21_reSampler[1]));
#endif

      assert(IIR21_reSampler[0].delay <= MAX_DS_FILTER_DELAY);

      coder->writeOffset += IIR21_reSampler[0].delay * MAX_CHANNELS;
    }
  }

  if (bDoIIR32Resample) {
    IIR32Init();
  }

  /*
     set up AAC encoder, now that samling rate is known
  */
  coder->_config.sampleRate = coder->sampleRateAAC;

  int ret = AacEncOpen(&coder->_enc, coder->_config);

  if (ret) {
    fprintf(stderr, "\n Initialisation of AAC failed !\n");
    goto l_error;
  }

  enc = coder;
  return 0;
l_error:

  if (coder) {
    close_aac_encoder(coder);
  }

  return -1;
}

int close_aac_encoder(struct aac_encoder_t *enc) {
  assert(enc);
  AacEncClose(enc->_enc);

  if (enc->_enc_env)
    EnvClose(enc->_enc_env);

  free(enc);
  return 0;
}

int aac_encode(struct aac_encoder_t *enc,
               const short pcm_data[AACENC_BLOCKSIZE * 2 * MAX_CHANNELS],
               int numSamplesRead) {

  for (int i = 0; i < numSamplesRead; ++ i) {    
    enc->resamplerScratch[i] = (float) pcm_data[i];
  }

  int nSamplesProcessed = 0;

  switch (enc->input_channls) {
  case 1:
    enc->nSamplesPerChannel = numSamplesRead;
    break;
  case 2:    
    enc->nSamplesPerChannel = numSamplesRead >> 1;
    break;
  default:    
    enc->nSamplesPerChannel = numSamplesRead / enc->input_channls;
  }

  nSamplesProcessed += IIR32Resample(enc->resamplerScratch,
    &enc->inputBuffer[enc->writeOffset + 1 * enc->nDSOutBlockSize * enc->stride],
    enc->nSamplesPerChannel, enc->nDSOutBlockSize, enc->input_channls);

  if (1 == enc->input_channls && enc->stride==2) {
    /* reordering necessary since the encoder takes interleaved data */
    for (int i = enc->nDSOutBlockSize - 1; i >= 0; -- i) {
      enc->inputBuffer[enc->writeOffset + 1 * enc->nDSOutBlockSize * 2 + 2 * i] = 
        enc->inputBuffer[enc->writeOffset+1 * enc->nDSOutBlockSize * 2 + i];
    }
  }

  numSamplesRead = nSamplesProcessed;


  return 0;
}