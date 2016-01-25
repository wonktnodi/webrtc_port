#ifndef __AAC_DECODER_DEF_H__A
#define __AAC_DECODER_DEF_H__A
#include "channelinfo2.h"
#include "channel.h"
struct AAC_DECODER_INSTANCE {
	unsigned char frameOK;   /*!< Will be unset if the CRC, a consistency check etc. fails */
	unsigned long bitCount;
	long byteAlignBits;
	HANDLE_BIT_BUF pBs;
	float *pTimeData;
	SBRBITSTREAM *pStreamSbr;
	CStreamInfo StreamInfo;
	CAacDecoderChannelInfo *pAacDecoderChannelInfo[Channels];
	CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo[Channels];
	int BlockNumber;

	int noise_index;
	int noise_left_index;
	int noise_left_index_start;
	AAC_DECODER_INSTANCE(){
		BlockNumber = 0;

		noise_index = 0;
		noise_left_index = 0;
		noise_left_index_start = 0;
	}
};

#endif //__AAC_DECODER_DEF_H__A
