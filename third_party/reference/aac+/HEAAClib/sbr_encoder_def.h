#ifndef __SBR_ENCODER_DEF_H__
#define __SBR_ENCODER_DEF_H__

struct SBR_ENCODER
{
	struct SBR_CONFIG_DATA    sbrConfigData;
	struct SBR_HEADER_DATA_ENC    sbrHeaderData;
	struct SBR_BITSTREAM_DATA sbrBitstreamData;
	struct ENV_CHANNEL*       hEnvChannel[MAX_CHANNELS];
	struct COMMON_DATA        CmonData;

	struct PS_ENC             PsEnc;
	struct PS_ENC             *hPsEnc;
	SBR_QMF_FILTER_BANK_ENC       *hSynthesisQmfBank;
	SBR_QMF_FILTER_BANK_ENC       synthesisQmfBank;
	unsigned int              sbrPayloadPrevious[MAX_PAYLOAD_SIZE/(sizeof(int))];
	unsigned int              sbrPayload[MAX_PAYLOAD_SIZE/(sizeof(int))];
	int                       sbrPayloadSize;
	float						sbr_envYBuffer[MAX_CHANNELS  * QMF_TIME_SLOTS * QMF_CHANNELS];

	SBR_ENCODER()
	{
		memset(&sbrConfigData, 0, sizeof(sbrConfigData));
		memset(&sbrHeaderData, 0, sizeof(sbrHeaderData));
		memset(&sbrBitstreamData, 0, sizeof(sbrBitstreamData));
		memset(&CmonData, 0 ,sizeof(CmonData));
		memset(&PsEnc, 0, sizeof(PsEnc));
		memset(&synthesisQmfBank, 0, sizeof(synthesisQmfBank));
		memset(sbrPayloadPrevious, 0, sizeof(sbrPayloadPrevious));
		memset(sbrPayload, 0, sizeof(sbrPayload));
		sbrPayloadSize = 0;
		memset(sbr_envYBuffer, 0, sizeof(sbr_envYBuffer));
	}
} ;

#endif//__SBR_ENCODER_DEF_H__