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

 $Id: aacdecoder.h,v 1.4.6.9 2004/05/30 14:11:13 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  decoder main  $Revision: 1.4.6.9 $
  \author zerok 
*/

#ifndef AACDECODER_H
#define AACDECODER_H

#include <stdio.h>
#include "channelinfo.h"
#include "sbrdecoder.h"
#include "FFR_bitbuffer.h"
#include "channelinfo2.h"


enum {
  AAC_DEC_OK = 0x0,
  AAC_DEC_UNSUPPORTED_FORMAT,
  AAC_DEC_DECODE_FRAME_ERROR,
  AAC_DEC_INVALID_CODE_BOOK,
  AAC_DEC_UNSUPPORTED_WINDOW_SHAPE,
  AAC_DEC_PREDICTION_NOT_SUPPORTED_IN_LC_AAC,
  AAC_DEC_UNIMPLEMENTED_PCE,
  AAC_DEC_UNIMPLEMENTED_DSE,
  AAC_DEC_UNIMPLEMENTED_LFE,
  AAC_DEC_UNIMPLEMENTED_CCE,
  AAC_DEC_UNIMPLEMENTED_GAIN_CONTROL_DATA,
  AAC_DEC_UNIMPLEMENTED_EP_SPECIFIC_CONFIG_PARSE,
  AAC_DEC_UNIMPLEMENTED_CELP_SPECIFIC_CONFIG_PARSE,
  AAC_DEC_UNIMPLEMENTED_HVXC_SPECIFIC_CONFIG_PARSE,
  AAC_DEC_OVERWRITE_BITS_IN_INPUT_BUFFER,
  AAC_DEC_CANNOT_REACH_BUFFER_FULLNESS
};


typedef struct AAC_DECODER_INSTANCE *AACDECODER;

#define FRAME_SIZE  1024


/* initialization of aac decoder */
AACDECODER CAacDecoderOpen(HANDLE_BIT_BUF pBs,
                           SBRBITSTREAM *streamSBR,
                           float *pTimeData,
						   float *pOverlapBuf,
						   void* aacDecoderStaticChannelInfo);

int CAacDecoderInit(AACDECODER self,
                    int samplingRate,
                    int bitrate);

void CAacDecoder_SetPara(AACDECODER self,
						 CAacDecoderDynamicCommonData*pAacDecoderDynamicCommonDataInit,
						 CAacDecoderDynamicData *pAacDecoderDynamicDataInit[],
						 CAacDecoderChannelInfo *pAacDecoderChannelInfoInit[],
						 float *workBufferCore);
/* aac decoder */
int CAacDecoder_DecodeFrame(AACDECODER aacDecoderInstance,
                            int *frameSize,
                            int *sampleRate,
                            int *numChannels,
                            char *channelMode,
                            char errorStatus
                            );


void CAacDecoderClose(AACDECODER pDec);

#endif /* #ifndef AACDECODER_H */
