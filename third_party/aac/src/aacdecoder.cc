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

 $Id: aacdecoder.c,v 1.1.2.9 2004/05/30 14:11:13 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  decoder main  $Revision: 1.1.2.9 $
  \author zerok 
*/

#include <assert.h>
#include "aac_rom_dec.h"
#include "aac_ram_dec.h"
#include "aacdecoder.h"
#include "FFR_bitbuffer.h"
#include <string>
#include "FFR_aacPLUScheck.h"

#include "aacdecoder_def.h"

#include "conceal.h"

#include "flc.h" /* the 3GPP instrumenting tool */

void CPns_InitPns(CAacDecoderChannelInfo *pAacDecoderChannelInfo);
void CPns_InitInterChannelData(CAacDecoderChannelInfo *pAacDecoderChannelInfo);


/*
  The function initializes the pointers to AacDecoderChannelInfo for each channel,
  set the start values for window shape and window sequence of overlap&add to zero,
  set the overlap buffer to zero and initializes the pointers to the window coefficients.
*/
AACDECODER CAacDecoderOpen(HANDLE_BIT_BUF pBs,       /*!< pointer to bitbuffer structure */
                           SBRBITSTREAM *pStreamSbr, /*!< pointer to sbr bitstream structure */
                           float *pTimeData,
						   float *overlapBuf,
						   void* pp)         /*!< pointer to time data */
{
  int i,ch;
  AAC_DECODER_INSTANCE* pAacDecoderInstance = new AAC_DECODER_INSTANCE;
  AAC_DECODER_INSTANCE &AacDecoderInstance = *pAacDecoderInstance;
  CAacDecoderStaticChannelInfo* aacDecoderStaticChannelInfo = ( CAacDecoderStaticChannelInfo*)pp;
  memset(pAacDecoderInstance, 0 , sizeof(AAC_DECODER_INSTANCE));
  
  FLC_sub_start("CAacDecoderOpen");

  /* initialize bit counter for syncroutine */
  MOVE(1);
  AacDecoderInstance.bitCount = 0;

  /* initialize pointer to bit buffer structure */
  MOVE(1);
  AacDecoderInstance.pBs = pBs;

  /* initialize pointer to time data */
  MOVE(1);
  AacDecoderInstance.pTimeData = pTimeData;

  /* initialize pointer to sbr bitstream structure */
  MOVE(1);
  AacDecoderInstance.pStreamSbr = pStreamSbr;

  /* initialize stream info */
  PTR_INIT(1); FUNC(1);
  CStreamInfoOpen(AacDecoderInstance.StreamInfo);

  /* initialize CAacDecoderChannelInfo */
  PTR_INIT(2); /* AacDecoderInstance.pAacDecoderStaticChannelInfo[ch] 
                  AacDecoderStaticChannelInfo[ch]
               */
  LOOP(1);
  for (ch=0; ch<Channels; ch++)
  {
    /* initialize pointer to CAacDecoderStaticChannelInfo for each channel */
    MOVE(1);
    AacDecoderInstance.pAacDecoderStaticChannelInfo[ch] = &aacDecoderStaticChannelInfo[ch];

    /* initialize overlap & add for each channel */
    MOVE(2);
    AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->OverlapAddData.WindowShape = 0;
    AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->OverlapAddData.WindowSequence = 0;

    PTR_INIT(1); MOVE(1);
    AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->OverlapAddData.pOverlapBuffer = &overlapBuf[ch*OverlapBufferSize];

    PTR_INIT(1); /* AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->OverlapAddData.pOverlapBuffer[i] */
    LOOP(1);
    for (i=0; i<FRAME_SIZE/2; i++)
    {
      MOVE(1);
      AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->OverlapAddData.pOverlapBuffer[i] = 0.0;
    }

    /* initialize window shapes for each channel */
    MOVE(4);
    AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->pLongWindow[0] = OnlyLongWindowSine;
    AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->pShortWindow[0] = OnlyShortWindowSine;
    AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->pLongWindow[1] = OnlyLongWindowKBD;
    AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->pShortWindow[1] = OnlyShortWindowKBD;

    PTR_INIT(1); MOVE(1);
    AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->hConcealmentInfo = 
								&AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]->ConcealmentInfo[ch];

    FUNC(1);
    CConcealment_Init (AacDecoderInstance.pAacDecoderStaticChannelInfo[ch]);
  }

  PTR_INIT(1); /* counting post-operation */

  FLC_sub_end();

  return &AacDecoderInstance;
}

int CAacDecoderInit(AACDECODER self,
                    int samplingRate,
                    int bitrate)
{
  int i;
  int numEntries = sizeof(SamplingRateInfoTable)/sizeof(SamplingRateInfo);

  FLC_sub_start("CAacDecoderInit");

  MOVE(1); /* counting previous operation */

  BRANCH(1);
  if (!self)
  {
    FLC_sub_end();
    return -1;
  }

  INDIRECT(1); MOVE(1);
  self->StreamInfo.SamplingRate  = samplingRate;
  
  PTR_INIT(1); /* SamplingRateInfoTable[i] */
  LOOP(1);
  for (i=0; i<numEntries; i++)
  {
    ADD(1); BRANCH(1);
    if (samplingRate == SamplingRateInfoTable[i].SamplingFrequency)
      break;
  }

  ADD(1); BRANCH(1);
  if (i == numEntries)
  {
    FLC_sub_end();
    return -1;
  }

  MOVE(2);
  self->StreamInfo.SamplingRateIndex = i;
  self->StreamInfo.BitRate = bitrate;

  FLC_sub_end();
  
  return 0;
}

void CAacDecoder_SetPara(AACDECODER self,
						 CAacDecoderDynamicCommonData*pAacDecoderDynamicCommonDataInit,
						 CAacDecoderDynamicData *pAacDecoderDynamicDataInit[Channels],
						 CAacDecoderChannelInfo *pAacDecoderChannelInfoInit[Channels],
						 float *workBufferCore)
{
	if(!self)
		return;

	for (int ch=0; ch<Channels; ch++)
	{
		INDIRECT(1); PTR_INIT(5);
		self->pAacDecoderChannelInfo[ch] = pAacDecoderChannelInfoInit[ch];

		self->pAacDecoderChannelInfo[ch]->pCodeBook = pAacDecoderDynamicDataInit[ch]->aCodeBook;
		self->pAacDecoderChannelInfo[ch]->pScaleFactor = pAacDecoderDynamicDataInit[ch]->aScaleFactor;

		self->pAacDecoderChannelInfo[ch]->pJointStereoData = &pAacDecoderDynamicCommonDataInit->JointStereoData;
		self->pAacDecoderChannelInfo[ch]->pSpectralCoefficient = &workBufferCore[ch*MaximumBinsLong];
	}
}

/*!
  The function decodes one aac frame. The decoding of coupling channel
  elements are not supported. The transport layer might signal, that the
  data of the current frame is invalid, e.g. as a result of a packet
  loss in streaming mode.
*/
int CAacDecoder_DecodeFrame(AACDECODER self,            /*!< pointer to aacdecoder structure */
                            int *frameSize,             /*!< pointer to frame size */
                            int *sampleRate,            /*!< pointer to sample rate */
                            int *numChannels,           /*!< pointer to number of channels */
                            char *channelMode,          /*!< mode: 0=mono, 1=stereo */
                            char frameOK                /*!< indicates if current frame data is valid */
                            )
{
  unsigned char aacChannels=0;
  long tmp;
  unsigned char ch;
  int type = 0;
  int ErrorStatus = frameOK;
  //static int BlockNumber = 0;
  HANDLE_BIT_BUF bs = self->pBs;
  int previous_element;
  int channel;

  FLC_sub_start("CAacDecoder_DecodeFrame");

  MOVE(7); INDIRECT(1); PTR_INIT(1); /* counting previous operations */

  /* initialize pointer to CAacDecoderChannelInfo for each channel */
  PTR_INIT(4); /* self->pAacDecoderChannelInfo[ch]
                  pAacDecoderChannelInfoInit[ch];
                  pAacDecoderDynamicDataInit[ch]
                  WorkBufferCore[ch*MaximumBinsLong];
               */
  LOOP(1);

  INDIRECT(1); MOVE(1);
  self->frameOK = 1;

  PTR_INIT(1); /* self->pAacDecoderChannelInfo[channel] */
  LOOP(1);
  for (channel=0; channel < Channels; channel++)
  {
    FUNC(1);
    CPns_InitPns(self->pAacDecoderChannelInfo[channel]);
  }

  INDIRECT(1); FUNC(1);
  CPns_InitInterChannelData(self->pAacDecoderChannelInfo[0]);

  MOVE(1);
  aacChannels = 0;

  INDIRECT(1); PTR_INIT(1); FUNC(2);
  ByteAlign(bs,&self->byteAlignBits);

  MOVE(1);
  previous_element = ID_END;

  LOOP(1);
  while ( (type != ID_END) && self->frameOK )
  {
    ADD(1); LOGIC(1); /* while() condition */

    FUNC(2);
    type = GetBits(bs,3);

    INDIRECT(1); BRANCH(1);
    if (bs->cntBits < 0)
    {
      INDIRECT(1); MOVE(1);
      self->frameOK = 0;
    }

    BRANCH(1);
    switch (type)
    {
      case ID_SCE:
      
        /*
          Consistency check
        */

        ADD(1); BRANCH(1);
        if(aacChannels >= Channels){

          INDIRECT(1); MOVE(1);
          self->frameOK = 0;
          break;
        }
          
        INDIRECT(1); MOVE(1);
        self->pStreamSbr->sbrElement[self->pStreamSbr->NrElements].ElementID = SBR_ID_SCE;

        INDIRECT(1); BRANCH(1); 
        if(self->frameOK) {

          INDIRECT(3); PTR_INIT(1); FUNC(3);
          ErrorStatus = CSingleChannelElement_Read(bs,self->pAacDecoderChannelInfo,&self->StreamInfo);

          BRANCH(1);
          if (ErrorStatus) {

            INDIRECT(1); MOVE(1);
            self->frameOK = 0;
          }
        }

        INDIRECT(1); BRANCH(1);
        if ( self->frameOK )
        {
          INDIRECT(1); FUNC(1);
          CSingleChannelElement_Decode(self,self->pAacDecoderChannelInfo);

          ADD(1);
          aacChannels += 1;
        }

        break;
      
      case ID_CPE:
       
        /*
          Consistency check
        */
        ADD(1); BRANCH(1);
        if(aacChannels >= Channels){

          INDIRECT(1); MOVE(1);
          self->frameOK = 0;
          break;
        }
          
        INDIRECT(1); MOVE(1);
        self->pStreamSbr->sbrElement[self->pStreamSbr->NrElements].ElementID = SBR_ID_CPE;


        INDIRECT(1); BRANCH(1);
        if (self->frameOK) {

          INDIRECT(3); PTR_INIT(1); FUNC(3);
          ErrorStatus = CChannelPairElement_Read(bs,self->pAacDecoderChannelInfo,&self->StreamInfo);

          BRANCH(1);
          if (ErrorStatus) {

            INDIRECT(1); MOVE(1);
            self->frameOK = 0;
          }
        }

        INDIRECT(1); BRANCH(1);
        if ( self->frameOK ){

          INDIRECT(1); FUNC(1);
          CChannelPairElement_Decode(self,self->pAacDecoderChannelInfo);

          ADD(1);
          aacChannels += 2;
        }

        break;
      
      case ID_CCE:
      
        INDIRECT(1); MOVE(2);
        ErrorStatus = AAC_DEC_UNIMPLEMENTED_CCE;
        self->frameOK = 0;
        break;

      case ID_LFE:

        INDIRECT(1); MOVE(2);
        ErrorStatus = AAC_DEC_UNIMPLEMENTED_LFE;
        self->frameOK = 0;
        break;
      
      case ID_DSE:
      
        INDIRECT(1); MOVE(2);
        ErrorStatus = AAC_DEC_UNIMPLEMENTED_DSE;
        self->frameOK = 0;
        break;
      
      case ID_PCE:

        INDIRECT(1); MOVE(2);
        ErrorStatus = AAC_DEC_UNIMPLEMENTED_PCE;
        self->frameOK = 0;

        break;
      
      case ID_FIL:

        INDIRECT(1); MOVE(1);
        tmp = bs->cntBits;

        FFRaacplus_checkForPayload(bs,self->pStreamSbr,previous_element);
        break;
      
      case ID_END:
        break;
    }

    MOVE(1);
    previous_element = type;
  }


  /* Update number of channels (if valid) */
  INDIRECT(1); BRANCH(1);
  if (self->frameOK) {

    INDIRECT(1); MOVE(1);
    self->StreamInfo.Channels = aacChannels;
  }

  INDIRECT(1); LOGIC(1); STORE(1);
  self->frameOK = self->frameOK && frameOK;

  /*
    Inverse transform
  */
  PTR_INIT(3); /* self->pAacDecoderStaticChannelInfo[ch]
                  self->pAacDecoderChannelInfo[ch]
                  self->pTimeData[ch*MaximumBinsLong]
               */
  INDIRECT(1); LOOP(1);
  for (ch=0; ch<self->StreamInfo.Channels; ch++) {

    /*
      Conceal erred spectral data 
    */
    FUNC(3);
    CConcealment_Apply(self->pAacDecoderStaticChannelInfo[ch],
                       self->pAacDecoderChannelInfo[ch],
                       self->frameOK);
  
  
    PTR_INIT(1); FUNC(1); BRANCH(1);
    if (IsLongBlock(&self->pAacDecoderChannelInfo[ch]->IcsInfo))
    {
      PTR_INIT(1); FUNC(4);
      CLongBlock_FrequencyToTime(self->pAacDecoderStaticChannelInfo[ch],
                                 self->pAacDecoderChannelInfo[ch],
                                 &self->pTimeData[ch*MaximumBinsLong],
                                 1);
    }
    else
    {
      PTR_INIT(1); FUNC(4);
      CShortBlock_FrequencyToTime(self->pAacDecoderStaticChannelInfo[ch],
                                  self->pAacDecoderChannelInfo[ch],
                                  &self->pTimeData[ch*MaximumBinsLong],
                                  1);
    }
  }


  INDIRECT(1); PTR_INIT(1); FUNC(2);
  ByteAlign(bs,&self->byteAlignBits);

  INDIRECT(3); MOVE(3);
  *frameSize = self->StreamInfo.SamplesPerFrame;
  *sampleRate = self->StreamInfo.SamplingRate;
  *numChannels = self->StreamInfo.Channels;

  ADD(1); BRANCH(1);
  if (*numChannels == 1)
  {
    MOVE(1);
    *channelMode = 0;
  }
  else {
    BRANCH(1); MOVE(1);
    if (*numChannels == 2)
      *channelMode = 1;
    else
      *channelMode = 2;
  }

  ADD(1);
  self->BlockNumber++;

  FLC_sub_end();

  return ErrorStatus;
}

void CAacDecoderClose(AACDECODER pDec)
{
	if(pDec)
		delete pDec;
}



