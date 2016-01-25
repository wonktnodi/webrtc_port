#ifndef  __HEAAC_ENC_H__
#define  __HEAAC_ENC_H__
#include <string>

class IHEAAC_ENC
{
public:
	virtual ~IHEAAC_ENC(){};

	virtual bool Init(unsigned int nSampleRate, unsigned int nChannels, unsigned int nBitrate) = 0;
	virtual int GetSampleCount() = 0;
	virtual bool Enc(const short* pwData, std::string& encData) = 0;
};

IHEAAC_ENC *CreateAACEnc();
void DestroyAACEnc(IHEAAC_ENC *p);

#endif

