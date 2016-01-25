/*
  Memory layout
*/

#ifndef AAC_RAM_H
#define AAC_RAM_H

#include "channel.h"


extern CConcealmentInfo ConcealmentInfo[Channels];

extern CStreamInfo StreamInfo;
extern CAacDecoderStaticChannelInfo AacDecoderStaticChannelInfo[Channels];

extern CPnsStaticInterChannelData PnsStaticInterChannelData;

extern float OverlapBuffer[Channels*OverlapBufferSize];

extern float WorkBufferCore[MaximumBinsLong*Channels];
extern CAacDecoderChannelInfo *pAacDecoderChannelInfoInit[Channels];
extern CAacDecoderDynamicData *pAacDecoderDynamicDataInit[Channels];

extern CAacDecoderDynamicCommonData *pAacDecoderDynamicCommonDataInit;

#endif /* #ifndef AAC_RAM_H */
