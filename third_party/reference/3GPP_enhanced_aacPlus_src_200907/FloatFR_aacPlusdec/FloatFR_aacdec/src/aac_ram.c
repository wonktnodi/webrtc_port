/*
  Memory layout
*/
#include <stdio.h>
#include "aacdecoder.h"
#include "aac_ram.h"
#include "FloatFR.h"

/*
  Static memory areas, must not be overwritten in other sections of the decoder
*/

/* The structure CAacDecoderStaticChannelInfo contains the static sideinfo which is needed
   for the decoding of one aac channel. Dimension: #AacDecoderChannels */
CAacDecoderStaticChannelInfo AacDecoderStaticChannelInfo[Channels];

/* The structure CStreamInfo contains the streaming information. */
CStreamInfo StreamInfo;

/* The structure CPnsStaticInterChannelData contains the random number generator state and the pns frame counter */
CPnsStaticInterChannelData PnsStaticInterChannelData;

/* Overlap buffer */
float OverlapBuffer[Channels*OverlapBufferSize];

CConcealmentInfo ConcealmentInfo[Channels];



/*!
  Dynamic memory areas, might be reused in other algorithm sections,
  e.g. the sbr decoder
*/

/* AAC Decoder Workbuffer */
float WorkBufferCore[Channels*MaximumBinsLong];
extern float TimeDataFloat[4*MaximumBinsLong]; /* allocated in main.c */

#ifdef LP_SBR_ONLY
CAacDecoderChannelInfo AacDecoderChannelInfo[Channels];

CAacDecoderChannelInfo *pAacDecoderChannelInfoInit[Channels] =
{
  &AacDecoderChannelInfo[0],
  &AacDecoderChannelInfo[1]
};
#else
extern float InterimResult[MaximumBinsLong];

CAacDecoderChannelInfo *pAacDecoderChannelInfoInit[Channels] =
{
  (CAacDecoderChannelInfo *) &InterimResult[0*sizeof(CAacDecoderChannelInfo)/sizeof(float)],
  (CAacDecoderChannelInfo *) &InterimResult[1*sizeof(CAacDecoderChannelInfo)/sizeof(float)]
};
#endif


CAacDecoderDynamicData *pAacDecoderDynamicDataInit[Channels] =
{
  (CAacDecoderDynamicData *) &TimeDataFloat[3*MaximumBinsLong],
  (CAacDecoderDynamicData *) &TimeDataFloat[3*MaximumBinsLong+sizeof(CAacDecoderDynamicData)/sizeof(float)]
};

CAacDecoderDynamicCommonData *pAacDecoderDynamicCommonDataInit =
  (CAacDecoderDynamicCommonData *) &TimeDataFloat[3*MaximumBinsLong+2*sizeof(CAacDecoderDynamicData)/sizeof(float)];

