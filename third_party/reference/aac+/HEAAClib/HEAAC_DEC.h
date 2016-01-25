#ifndef __HEAAC_DEC_H__
#define __HEAAC_DEC_H__
#include <string>

class IHEAAC_DEC
{
public:
	virtual bool Init(int nSampleRate, int nBitrate) = 0;
	virtual bool Dec(const std::string& decData, std::string& pcmData, bool frameOk) = 0;

	virtual ~IHEAAC_DEC(){}
};

IHEAAC_DEC *CreateAACDec();
void DestroyAACDec(IHEAAC_DEC *p);

#endif //__HEAAC_DEC_H__

