/****************************************************************************

(C) copyright Coding Technologies (2003)
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

$Id: main.c,v 1.1.2.27 2004/05/30 14:00:54 ehr Exp $

****************************************************************************/



#if 0
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "FloatFR.h"
#include "aacenc.h"

/* for MPEG-4 file format support */
#include "mp4file.h"
#include "ISOMovies.h"

#include "sbr_main.h"
#include "aac_ram_enc.h"
#include "aac_rom_enc.h"

#include "FFR_bitbuffer.h"
#include "../../HEAACDec/FloatFR_aacdec/aacdecoder.h"

/* dynamic buffer of SBR that can be reused for resampling */
extern float sbr_envRBuffer[];

#define CORE_DELAY   (1600)
#define INPUT_DELAY  ((CORE_DELAY)*2 +6*64-2048+1)     /* ((1600 (core codec)*2 (multi rate) + 6*64 (sbr dec delay) - 2048 (sbr enc delay) + magic*/
#define MAX_DS_FILTER_DELAY 16                         /* the additional max resampler filter delay (source fs)*/

#define CORE_INPUT_OFFSET_PS (0)  /* (96-64) makes AAC still some 64 core samples too early wrt SBR ... maybe -32 would be even more correct, but 1024-32 would need additional SBR bitstream delay by one frame */

#define SAMPLES_PER_FRAME 1024

 float TimeDataFloat[4*SAMPLES_PER_FRAME];
/*
input buffer (1ch)
|------------ 1537   -------------|-----|---------- 2048 -------------|
(core2sbr delay     )          ds     (read, core and ds area)
*/
static float inputBuffer[(AACENC_BLOCKSIZE*2 + MAX_DS_FILTER_DELAY + INPUT_DELAY)*MAX_CHANNELS];
unsigned int outputBuffer[(6144/8)*MAX_CHANNELS/(sizeof(int))];

__inline void interleaveSamples(float *pTimeCh0, 
								float *pTimeCh1, short *pTimeOut, 
								int frameSize, int *channels)
{
	for (int i=0; i<frameSize; i++)
	{
		*pTimeOut++ = (short) *pTimeCh0++;

		if(*channels == 2) {
			*pTimeOut++ = (short) *pTimeCh1++;
		}
		else {
			*pTimeOut = *(pTimeOut-1);
			*pTimeOut++;
		}
	}

	*channels = 2;
}

int main(/*int argc, char *argv[]*/)
{
	AACENC_CONFIG     config;
	FILE* inputFile = NULL;

	SBRDECODER sbrDecoderInfo = 0;

	FILE *fOut=NULL;
	HANDLE_MP4_FILE hMp4File;

	int  error;
	int bitrate;
	int nChannelsAAC, nChannelsSBR;
	unsigned int sampleRateAAC;
	int frmCnt;
	int bandwidth = 0;

	unsigned int numAncDataBytes=0;
	unsigned char ancDataBytes[MAX_PAYLOAD_SIZE];

	/*!< required only for interfacing with audio output library, thus not counted into RAM requirements */
	short TimeDataPcm[AACENC_BLOCKSIZE*2*MAX_CHANNELS];

	int numSamplesRead;
	int bDoIIR2Downsample = 0;
	int bDingleRate = 0;
	int useParametricStereo = 0;
	int coreWriteOffset = 0;
	int coreReadOffset = 0;
	int envWriteOffset = 0;
	int envReadOffset = 0;
	int writeOffset=INPUT_DELAY*MAX_CHANNELS;

	struct AAC_ENCODER *aacEnc = 0;

	int bDoUpsample = 0;
	int upsampleReadOffset = 0;

	int inSamples_enc;
	int bDoIIR32Resample = 0;
	const int nRuns = 4;
	float *resamplerScratch = sbr_envRBuffer;

	HANDLE_SBR_ENCODER hEnvEnc=NULL;

	bitrate = 24000;

	/*open audio input file*/
	int sampleRate     = 48000 ;    /* only relevant if valid == 1 */
	int nChannels      = 2 ;        /* only relevant if valid == 1 */

	inputFile = fopen("f:\\xxxx.wav", "rb");
	if(inputFile == NULL)
	{
		assert(0);
		return 0;
	}

	/* set up basic parameters for aacPlus codec*/
	AacInitDefaultConfig(&config);

	nChannelsAAC = nChannelsSBR = nChannels;
	if ( (nChannels == 2) && (bitrate > 16000) && (bitrate < 36000) )
	{
		useParametricStereo = 1;
	}

	if (useParametricStereo)
	{
		nChannelsAAC = 1;
		nChannelsSBR = 2;
	}

	if ( (sampleRate == 48000) && (nChannelsAAC == 2) && (bitrate < 24000) ) {
		bDoIIR32Resample  = 1;
	}

	if (sampleRate == 16000) {
		bDoUpsample = 1;
		sampleRate = 32000;
		bDingleRate = 1;
	}

	sampleRateAAC = sampleRate;

	if (bDoIIR32Resample)
		sampleRateAAC = 32000;

	config.bitRate = bitrate;
	config.nChannelsIn=nChannels;
	config.nChannelsOut=nChannelsAAC;

	config.bandWidth=bandwidth;
	/*set up SBR configuration*/
	if(!IsSbrSettingAvail (bitrate, nChannelsAAC, sampleRateAAC, &sampleRateAAC)) {
		fprintf(stderr,"No valid SBR configuration found\n");
		exit(10);
	}

	{
		sbrConfiguration sbrConfig;

		envReadOffset = 0;
		coreWriteOffset = 0;

		if(useParametricStereo)
		{
			envReadOffset = (MAX_DS_FILTER_DELAY + INPUT_DELAY)*MAX_CHANNELS;
			coreWriteOffset = CORE_INPUT_OFFSET_PS;
			writeOffset = envReadOffset;
		}

		InitializeSbrDefaults (&sbrConfig);

		sbrConfig.usePs = useParametricStereo;

		AdjustSbrSettings(&sbrConfig,
			bitrate,
			nChannelsAAC,
			sampleRateAAC,
			AACENC_TRANS_FAC,
			24000);

		EnvOpen(&hEnvEnc,
			inputBuffer + coreWriteOffset,
			&sbrConfig,
			&config.bandWidth);

		/* set IIR 2:1 downsampling */
		bDoIIR2Downsample = (bDoUpsample) ? 0 : 1;

		if (useParametricStereo)
		{
			bDoIIR2Downsample = 0;
		}else{
			assert(0);
		}

	}

	if (bDoUpsample) {
		assert(0);
	}
	else {
		if (bDoIIR2Downsample){
			assert(0);
		}
	}

	/*
	set up AAC encoder, now that samling rate is known
	*/
	config.sampleRate = sampleRateAAC;
	error = AacEncOpen( &aacEnc,config);

	if (error){
		assert(0);
		AacEncClose(aacEnc);
		fclose(inputFile);

		if (fOut) 
			fclose(fOut);
		return 1;
	}


	/*set up MPEG-4/3GPP file format library (not instrumented nor accounted for RAM requirements)*/
	{
		unsigned char ASConfigBuffer[80];
		unsigned int  nConfigBits;
		unsigned int  nConfigBytes;

		memset (ASConfigBuffer, 0, 80);

		if ( GetMPEG4ASConfig( sampleRateAAC,
			nChannelsAAC,
			ASConfigBuffer,
			&nConfigBits,
			1,
			bDingleRate) ) {
				fprintf(stderr, "\nCould not initialize Audio Specific Config\n");
				exit(10);
		}


		nConfigBytes = (nConfigBits+7)>>3;

		if (OpenMP4File(&hMp4File,
			ASConfigBuffer,
			nConfigBytes,
			"f:/out2.3gp",
			(!bDingleRate) ? sampleRateAAC*2 : sampleRateAAC, /* output sampleRate */
			config.bitRate,
			nChannelsAAC,
			1,
			1) ) {
				fprintf(stderr, "\nFailed to create 3GPP file\n") ;
				exit(10);
		}
	}

	frmCnt = 0;
	memset(TimeDataPcm,0,sizeof(TimeDataPcm));

	/*set up input samples block size feed*/
	inSamples_enc = AACENC_BLOCKSIZE * nChannels * 2; 


////decodec////////////////////////////////////////////////////////////////////////////////////////
	#define INPUT_BUF_SIZE (6144*2/8)                      /*!< Size of Input buffer in bytes*/
	unsigned int inBuffer[INPUT_BUF_SIZE/(sizeof(int))];   /*!< Input buffer */
	struct BIT_BUF bitBuf, *hBitBuf;
	AACDECODER aacDecoderInfo = 0; 
	SBRBITSTREAM streamSBR;   
	hBitBuf = CreateBitBuffer(&bitBuf,(unsigned char*) inBuffer,INPUT_BUF_SIZE);


	aacDecoderInfo = CAacDecoderOpen(hBitBuf,
		&streamSBR,
		TimeDataFloat);
	if(0 == aacDecoderInfo)
	{
		assert(0);
		return 0;
	}

	if(0 != CAacDecoderInit (aacDecoderInfo,sampleRate/2,bitrate))
	{
		assert(0);
		return 0;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
	FILE* fw = fopen("f:\\uuu.pcm", "wb");
	/*	The frame loop */
	while (1)
	{
		int i, numOutBytes;

		numSamplesRead = fread(TimeDataPcm, 2, inSamples_enc, inputFile);
		if(numSamplesRead <= 0)
		{
			break;
		}

		/* copy from short to float input buffer */
		if ( nChannels == nChannelsSBR ) {
			for (i=0; i<numSamplesRead; i++) {
				inputBuffer[i+writeOffset] = (float) TimeDataPcm[i];
			}
		}

#if (MAX_CHANNELS==2)
		/* copy from short to float input buffer, reordering necessary since the encoder takes interleaved data */
		if(nChannels == 1) {
			int i;
			for (i=0; i<numSamplesRead; i++) {
				inputBuffer[writeOffset+2*i] = (float) TimeDataPcm[i];
			}
		}
#endif

		/*encode one SBR frame*/
		EnvEncodeFrame (hEnvEnc,
			inputBuffer + envReadOffset,
			inputBuffer + coreWriteOffset,
			MAX_CHANNELS,
			&numAncDataBytes,
			ancDataBytes);

		/*encode one AAC frame*/
		if (hEnvEnc && useParametricStereo) {

			AacEncEncode(aacEnc,
				inputBuffer,
				1, /* stride */
				ancDataBytes,
				&numAncDataBytes,
				outputBuffer,
				&numOutBytes);

			if(hEnvEnc)
			{
				memcpy( inputBuffer,inputBuffer+AACENC_BLOCKSIZE,CORE_INPUT_OFFSET_PS*sizeof(float));
			}
		}
		else
		{
			assert(0);
		}

		////////////////////////////////////////dec
		unsigned char* pData = (unsigned char*)outputBuffer;
		for(int i = 0; i < numOutBytes; i++)
			WriteBits(hBitBuf,pData[i],8);

		streamSBR.NrElements = 0;
		int frameSize_dec= 0, sampleRate_dec = 0, numChannels_dec = 2;
		char frameOk_dec = 1, channelMode_dec = 0;
		int ErrorStatus = CAacDecoder_DecodeFrame(aacDecoderInfo,&frameSize_dec,&sampleRate_dec,
									&numChannels_dec,	&channelMode_dec,	frameOk_dec);

		assert(ErrorStatus == 0);
		if ((!sbrDecoderInfo) && streamSBR.NrElements)
			sbrDecoderInfo = openSBR (sampleRate_dec,frameSize_dec, 0,0);

		if (sbrDecoderInfo) {

			/* apply SBR processing */
			if (applySBR(sbrDecoderInfo, &streamSBR, TimeDataFloat,	&numChannels_dec,
						frameOk_dec, 0, 0) != SBRDEC_OK){
					sbrDecoderInfo = 0;
			}
			else {
				frameSize_dec = frameSize_dec * 2;
				sampleRate_dec *= 2;
			}
		}

		short* pwData = new short[frameSize_dec * 2];
		/* clip time samples */
		for (i = 0; i < frameSize_dec * numChannels_dec; i++) {
			if (TimeDataFloat[i] < -32768.0) {
				TimeDataFloat[i] = -32768.0;
			}
			else {
				if (TimeDataFloat[i] > 32767.0) {
					TimeDataFloat[i] = 32767.0;
				}
			}

			//pwData[i] = TimeDataFloat[i];
		}

		interleaveSamples(&TimeDataFloat[0],&TimeDataFloat[frameSize_dec],pwData,frameSize_dec,&numChannels_dec);

		fwrite(pwData, 2, frameSize_dec * numChannels_dec, fw);
		delete [] pwData;
////////////////////////////////////////////////////////////////////////
		if (numOutBytes) {
			MP4FileAddFrame( hMp4File, 
				outputBuffer,
				numOutBytes );
		}

		frmCnt++;

		/* 3GPP instrumenting tool: measure worst case work load at end of each decoding loop */
		fprintf(stderr,"[%d]\r",frmCnt); fflush(stderr);
	}
	fprintf(stderr,"\n"); fflush(stderr);

	AacEncClose(aacEnc);

	fclose(inputFile);

	if (WriteMP4File( hMp4File)) {
		fprintf(stderr, "Writing of 3GPP file failed.");
		exit(10);
	}
	CloseMP4File( hMp4File);

	if(hEnvEnc)
	{
		EnvClose(hEnvEnc);
	}

	printf("\nencoding finished\n");
	return 0;
}

#else



#include "../HEAAClib/HEAAC_ENC.h"
#include "../HEAAClib/HEAAC_DEC.h"
#include <assert.h>

int main()
{
	FILE* inputFile = NULL;

	FILE *fOut=NULL;
	int frmCnt;
	int bandwidth = 0;

	//unsigned char ancDataBytes[MAX_PAYLOAD_SIZE];
	short TimeDataPcm[1024*2*2];

	int numSamplesRead;
	int bDoIIR2Downsample = 0;
	int bDingleRate = 0;

	int inSamples_enc;
	const int nRuns = 4;

	/*open audio input file*/
	inputFile = fopen("f:\\xxxx.pcm", "rb");
	if(inputFile == NULL)
	{
		assert(0);
		return 0;
	}

	IHEAAC_ENC* enc = CreateAACEnc();
	IHEAAC_DEC* dec = CreateAACDec();
	IHEAAC_DEC* dec2 = CreateAACDec();
	IHEAAC_DEC* dec3 = CreateAACDec();

	assert(enc && enc->Init(48000, 2, 32000));
	assert(dec && dec->Init(48000, 32000));
	assert(dec2 && dec2->Init(48000, 32000));
	assert(dec3 && dec3->Init(48000, 32000));

	unsigned int sampleRateAAC = 48000/2;
	
	frmCnt = 0;
	FILE* fw = fopen("f:\\uuu.pcm", "wb");
	
	/*	The frame loop */
	while (1)
	{
		inSamples_enc = enc->GetSampleCount();
		numSamplesRead = fread(TimeDataPcm, 2, inSamples_enc, inputFile);
		if(numSamplesRead <= 0)
		{
			break;
		}

		/* copy from short to float input buffer */
		std::string xxx;
		enc->Enc(TimeDataPcm, xxx);

		std::string __pcmData;
		dec->Dec(xxx, __pcmData, true);
		std::string pcmData2;
  		dec2->Dec(xxx,pcmData2,true);
  		int iii = memcmp(__pcmData.data(), pcmData2.data(), __pcmData.size());
		assert(iii == 0);

		std::string pcmData3;
		dec3->Dec(xxx, pcmData3,true);
		iii = memcmp(pcmData2.data(), pcmData3.data(), __pcmData.size());
		assert(iii == 0);
 		fwrite((char*)__pcmData.data(),1, __pcmData.size(), fw);

		frmCnt++;

		/* 3GPP instrumenting tool: measure worst case work load at end of each decoding loop */
		fprintf(stderr,"[%d]\r",frmCnt); fflush(stderr);
	}
	fprintf(stderr,"\n"); fflush(stderr);
	fclose(inputFile);

	printf("\nencoding finished\n");

	DestroyAACDec(dec);
	DestroyAACEnc(enc);
	return 0;
}


#endif
