#include "HEAAC_ENC.h"
#include "aacenc.h"
#include <assert.h>
#include "sbr_main.h"
#include <stdio.h>


#define CORE_DELAY   (1600)
#define INPUT_DELAY  ((CORE_DELAY)*2 +6*64-2048+1)     
#define MAX_DS_FILTER_DELAY 16                         

#define CORE_INPUT_OFFSET_PS (0)  


class CHEAAC_ENC:public IHEAAC_ENC
{
public:
	CHEAAC_ENC():m_writeOffset(INPUT_DELAY*MAX_CHANNELS)
		,m_hEnvEnc(NULL)
		,m_pAacEnc(NULL)
		,m_inSamples_enc(0)
	{
		memset(m_inputBuffer, 0 ,sizeof(m_inputBuffer));
	}

	virtual ~CHEAAC_ENC()
	{
		if(m_pAacEnc)
			AacEncClose(m_pAacEnc);

		if(m_hEnvEnc)
		{
			EnvClose(m_hEnvEnc);
		}
	}

	virtual bool Init(unsigned int nSampleRate, unsigned int nChannels, unsigned int nBitrate)
	{
		int nChannelsAAC, nChannelsSBR;
		int useParametricStereo = 0;
		int bDoIIR32Resample = 0;

		assert(nChannels == 2);
		if(nChannels != 2)
			return false;

		AacInitDefaultConfig(&m_encConfig);

		nChannelsAAC = nChannelsSBR = nChannels;
		if ( (nChannels == 2) && (nBitrate >= 16000) && (nBitrate < 36000))
		{
			useParametricStereo = 1;
		}

		if (useParametricStereo)
		{
			nChannelsAAC = 1;
			nChannelsSBR = 2;
		}

		if ( (nSampleRate == 48000) && (nChannelsAAC == 2) && (nBitrate < 24000)) 
		{
			bDoIIR32Resample  = 1;
		}

		if (nSampleRate == 16000) 
		{
			assert(0);
			return false;
		}

		unsigned int sampleRateAAC = nSampleRate;

		if (bDoIIR32Resample)
			sampleRateAAC = 32000;

		m_encConfig.bitRate       = nBitrate;
		m_encConfig.nChannelsIn   = nChannels;
		m_encConfig.nChannelsOut  = nChannelsAAC;
		m_encConfig.bandWidth =  0; ///bandwidth;

		/*set up SBR configuration*/
		if(!IsSbrSettingAvail (nBitrate, nChannelsAAC, sampleRateAAC, &sampleRateAAC)) 
		{
			assert(0);
			return false;
		}

		sbrConfiguration sbrConfig;

		m_envReadOffset   = 0;
		m_coreWriteOffset = 0;

		if(useParametricStereo)
		{
			m_envReadOffset   = (MAX_DS_FILTER_DELAY + INPUT_DELAY)*MAX_CHANNELS;
			m_coreWriteOffset = CORE_INPUT_OFFSET_PS;
			m_writeOffset     = m_envReadOffset;
		}

		InitializeSbrDefaults (&sbrConfig);

		sbrConfig.usePs = useParametricStereo;
		AdjustSbrSettings(&sbrConfig, nBitrate, nChannelsAAC, sampleRateAAC, AACENC_TRANS_FAC, 24000);
		EnvOpen(&m_hEnvEnc, m_inputBuffer + m_coreWriteOffset, &sbrConfig, &m_encConfig.bandWidth);

		//only support ParametricStereo
		if(!useParametricStereo)
		{
			assert(0);
		}

		//aac init
		m_encConfig.sampleRate = sampleRateAAC;
		if(AacEncOpen(&m_pAacEnc,m_encConfig))
		{
			assert(0);
			AacEncClose(m_pAacEnc);

			return false;
		}

		/*set up input samples block size feed*/
		m_inSamples_enc = AACENC_BLOCKSIZE * nChannels * 2; 
		return true;
	}

	virtual bool Enc(const short* pwData, std::string& encData)
	{
		unsigned char ancDataBytes[MAX_PAYLOAD_SIZE];
		unsigned int numAncDataBytes = 0;
		unsigned int outputBuffer[(6144/8)*MAX_CHANNELS/(sizeof(int))];
		int numOutBytes = 0;
		if(!m_hEnvEnc)
		{
			return false;
		}

		for(register int i=0; i<m_inSamples_enc; i++) 
		{
			m_inputBuffer[i+m_writeOffset] = (float) pwData[i];
		}

		EnvEncodeFrame(m_hEnvEnc,
					   m_inputBuffer + m_envReadOffset,
					   m_inputBuffer + m_coreWriteOffset,
			           MAX_CHANNELS, &numAncDataBytes, ancDataBytes);

		/*encode one AAC frame*/
		AacEncEncode(m_pAacEnc, m_inputBuffer, 1, ancDataBytes, &numAncDataBytes, outputBuffer, &numOutBytes);
		memcpy(m_inputBuffer, m_inputBuffer+AACENC_BLOCKSIZE, CORE_INPUT_OFFSET_PS*sizeof(float));

		encData.assign((char*)outputBuffer,numOutBytes);

		return true;
	}

	virtual int GetSampleCount()
	{
		return m_inSamples_enc;
	}
private:
	AACENC_CONFIG        m_encConfig;
	int                  m_envReadOffset;
	int                  m_coreWriteOffset;
	int                  m_writeOffset;
	HANDLE_SBR_ENCODER   m_hEnvEnc;
	float                m_inputBuffer[(AACENC_BLOCKSIZE*2 + MAX_DS_FILTER_DELAY + INPUT_DELAY)*MAX_CHANNELS];

	struct AAC_ENCODER * m_pAacEnc;
	int                  m_inSamples_enc;
};


IHEAAC_ENC *CreateAACEnc()
{
	return new CHEAAC_ENC;
}

void DestroyAACEnc(IHEAAC_ENC *p)
{
	if(p)
		delete p;
}
