#include "HEAAC_DEC.h"
#include "FFR_bitbuffer.h"
#include <assert.h>
#include <stdio.h>
#include "sbr_dec.h"


#include "sbrdecoder.h"
#include "aac_ram_dec.h"
#include "aac_rom_dec.h"
#include "aacdecoder.h"
// IO-Buffers 
#define INPUT_BUF_SIZE (6144*2/8)                      
#define SAMPLES_PER_FRAME 1024

struct SBR_DECODER_INSTANCE
{
	SBR_CHANNEL      SbrChannel[MAXNRSBRCHANNELS];
	SBR_HEADER_DATA_DEC  sbr_header;
	struct PS_DEC ParametricStereoDec;
	SBR_CONCEAL_DATA SbrConcealData;
	float            InterimResult[MAX_FRAME_SIZE];   
	float            OverlapBuffer[Channels*OverlapBufferSize];

	SBR_DECODER_INSTANCE()
	{
		memset(SbrChannel, 0, sizeof(SbrChannel));
		memset(&sbr_header, 0, sizeof(sbr_header));
		memset(&ParametricStereoDec, 0, sizeof(ParametricStereoDec));
		memset(&SbrConcealData, 0, sizeof(SbrConcealData));
		memset(InterimResult, 0, sizeof(InterimResult));
		memset(OverlapBuffer, 0, sizeof(OverlapBuffer));
	}
};


class CHEAAC_DEC:public IHEAAC_DEC
{
public:
	CHEAAC_DEC():m_hBitBuf(0)
		,m_pAacDecoderInfo(NULL)
		,m_pSbrDecoderInfo(NULL)
	{

		m_delayPack.resize(1024*2 *4);
		memset(m_arrWorkBufferCore, 0, sizeof(m_arrWorkBufferCore));

		m_pAacDecoderDynamicDataInit[0] = (CAacDecoderDynamicData *) &m_arrTimeDataFloat[3*MaximumBinsLong];
		m_pAacDecoderDynamicDataInit[1] = (CAacDecoderDynamicData *) &m_arrTimeDataFloat[3*MaximumBinsLong+sizeof(CAacDecoderDynamicData)/sizeof(float)];

		m_pAacDecoderDynamicCommonDataInit = (CAacDecoderDynamicCommonData *) &m_arrTimeDataFloat[3*MaximumBinsLong+2*sizeof(CAacDecoderDynamicData)/sizeof(float)];

		//extern float InterimResult[];
		m_pAacDecoderChannelInfoInit[0] = (CAacDecoderChannelInfo *) &m_sbrDecoderInstance.InterimResult[0*sizeof(CAacDecoderChannelInfo)/sizeof(float)];
		m_pAacDecoderChannelInfoInit[1] = (CAacDecoderChannelInfo *) &m_sbrDecoderInstance.InterimResult[1*sizeof(CAacDecoderChannelInfo)/sizeof(float)];
	}

	virtual ~CHEAAC_DEC()
	{
		if(m_hBitBuf)
			DeleteBitBuffer(&m_hBitBuf);
	}

	bool Init(int nSampleRate, int nBitrate)
	{
		for (int i=0; i < 4*SAMPLES_PER_FRAME; i++)
		{
			m_arrTimeDataFloat[i] = 0.0;
		}

		/* initialize bit buffer */
		m_hBitBuf = CreateBitBuffer(&m_bitBuf,(unsigned char*)m_inBuffer, INPUT_BUF_SIZE);
		if(!m_hBitBuf)
		{
			assert(0); 
			return false;
		}

		/* initialize AAC decoder */
		m_pAacDecoderInfo = CAacDecoderOpen(m_hBitBuf,
			&m_streamSBR,
			m_arrTimeDataFloat,
			m_sbrDecoderInstance.OverlapBuffer,
			m_aacDecoderStaticChannelInfo);

		if (!m_pAacDecoderInfo) 
		{
			assert(0); 
			return false;
		}

		int nErrorNo = CAacDecoderInit(m_pAacDecoderInfo,nSampleRate/2,nBitrate);

		if (nErrorNo)
		{
			assert(0); 
			return false;
		}

		return true;
	}

	void interleaveSamples(float *pTimeCh0, float *pTimeCh1, short *pTimeOut, int frameSize, int *channels)
	{
		for(register int i=0; i<frameSize; i++)
		{
			*pTimeOut++ = (short) *pTimeCh0++;

			if(*channels == 2) 
			{
				*pTimeOut++ = (short) *pTimeCh1++;
			}
			else 
			{
				*pTimeOut = *(pTimeOut-1);
				*pTimeOut++;
			}
		}

		*channels = 2;
	}

	bool Dec(const std::string& decData, std::string& pcmData, bool frameOk)
	{
		/* decode one frame of audio data */
		m_streamSBR.NrElements = 0;

		//loss packet!!!!
		if(frameOk == false)
		{
			pcmData = m_delayPack;

			if(m_isLostPack == false)
			{
				m_isLostPack = true;
				const double fac = 1.0/ 4096.0;

				short* pwData = (short*) pcmData.data();
				for(int i = 0; i < 4096; i++)
					pwData[i] = fac *(4096-i) * pwData[i];

				memset((void*)m_delayPack.data(), 0, m_delayPack.size());
			}

			return true;
		}

		for (register int i=0; i<decData.size(); i++)
		{
			WriteBits(m_hBitBuf,decData[i], 8);
		}

		int frameSize = 0, sampleRate = 48000, numChannels = 2;
		char channelMode = 0;

		//SETUP DECODER PARAM
		CAacDecoder_SetPara(m_pAacDecoderInfo, m_pAacDecoderDynamicCommonDataInit ,m_pAacDecoderDynamicDataInit, m_pAacDecoderChannelInfoInit,m_arrWorkBufferCore);
		/* AAC core decoder */
		if (CAacDecoder_DecodeFrame(m_pAacDecoderInfo, &frameSize, &sampleRate, &numChannels, &channelMode,frameOk)) 
		{
			assert(0);
			return false;
		}

		// SBR decoder
		//open SBR-handle if SBR-Bitstream has been detected in core decoder
		if ((!m_pSbrDecoderInfo) && m_streamSBR.NrElements) 
		{
			m_pSbrDecoderInfo = openSBR(m_sbrDecoderInstance, sampleRate, frameSize, 0, 0);
		}

		if (m_pSbrDecoderInfo) 
		{
			//apply SBR processing
			if (applySBR(m_pSbrDecoderInfo,	&m_streamSBR,m_arrTimeDataFloat, m_arrWorkBufferCore,
				&numChannels,frameOk, 0,0) != SBRDEC_OK)
			{
					m_pSbrDecoderInfo = 0;
			}
		}

		frameSize = frameSize*2;
		sampleRate *= 2;

		int outputSampleRate = sampleRate;
		int numOutSamples = frameSize;

		pcmData = m_delayPack;
		short* pwData = (short*)m_delayPack.data();
		/* clip time samples */
		for (int i = 0; i < frameSize * numChannels; i++) 
		{
			if (m_arrTimeDataFloat[i] < -32768.0) 
			{
				m_arrTimeDataFloat[i] = -32768.0;
			}
			else 
			{
				if (m_arrTimeDataFloat[i] > 32767.0) 
				{
					m_arrTimeDataFloat[i] = 32767.0;
				}
			}
		}

		interleaveSamples(&m_arrTimeDataFloat[0],&m_arrTimeDataFloat[frameSize],pwData,frameSize,&numChannels);

		if(m_isLostPack)
		{
			const double fac = 1.0/ 4096.0;
			for(int i = 0; i < 4096; i++)
				pwData[i] = fac *i* pwData[i];

			m_isLostPack = false;
		}

		return true;
	}
private:
	struct BIT_BUF *m_hBitBuf,m_bitBuf;;

	unsigned int m_inBuffer[INPUT_BUF_SIZE/(sizeof(int))];  

	AACDECODER    m_pAacDecoderInfo;                   
	SBRBITSTREAM  m_streamSBR;                         
	SBRDECODER    m_pSbrDecoderInfo;                    
	bool		  m_isLostPack;
	std::string   m_delayPack;

	CAacDecoderStaticChannelInfo m_aacDecoderStaticChannelInfo[Channels];

	CAacDecoderChannelInfo *m_pAacDecoderChannelInfoInit[Channels];
	CAacDecoderDynamicData *m_pAacDecoderDynamicDataInit[Channels];
	CAacDecoderDynamicCommonData *m_pAacDecoderDynamicCommonDataInit;
	float m_arrTimeDataFloat[4*SAMPLES_PER_FRAME];

	float m_arrWorkBufferCore[MaximumBinsLong*Channels];
	SBR_DECODER_INSTANCE m_sbrDecoderInstance;
};


IHEAAC_DEC *CreateAACDec()
{
	return new CHEAAC_DEC();
}

void DestroyAACDec(IHEAAC_DEC *p)
{
	if(p != NULL)
	{
		delete p;
	}
}
