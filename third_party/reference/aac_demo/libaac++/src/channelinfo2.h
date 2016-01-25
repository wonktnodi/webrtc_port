#ifndef __CHANNEL_INFO_H__
#define __CHANNEL_INFO_H__

#define MAX_WINDOWS   8
#define MAX_SFB_LONG  64
#define MAX_SFB_SHORT 16
#define MAX_QUANTIZED_VALUE 8191
#define PNS_BAND_FLAGS_SIZE              8
enum
{
	MaximumWindows = 8,
	MaximumBands = 49,
	MaximumOrder = 31,
	MaximumFilters = 3
};

enum
{
	JointStereoMaximumGroups = 8,
	JointStereoMaximumBands = 64
};

typedef struct
{
	char MsMaskPresent; 
	unsigned char MsUsed[JointStereoMaximumBands]; /*!< every byte contains flags for up to 8 groups */
} CJointStereoData;

typedef struct
{
	CJointStereoData JointStereoData; /*! Common MS-mask for a channel-pair */
} CAacDecoderDynamicCommonData;

typedef struct
{
	short aScaleFactor[MAX_WINDOWS * MAX_SFB_SHORT];
	char  aCodeBook[MAX_WINDOWS * MAX_SFB_SHORT];

} CAacDecoderDynamicData;

typedef struct
{
	unsigned char Valid;

	char IcsReservedBit;
	char WindowShape;
	char WindowSequence;
	char MaxSfBands;
	char ScaleFactorGrouping;

	char TotalSfBands;
	char SamplingRateIndex;
	char Profile;

	char WindowGroups;
	char WindowGroupLength[MAX_WINDOWS];

} CIcsInfo;

typedef struct
{
	char StartBand;
	char StopBand;

	char Direction;
	char Resolution;

	char Order;
	char Coeff[MaximumOrder];
} CFilter;


typedef struct
{
	char TnsDataPresent;
	char NumberOfFilters[MaximumWindows];
	CFilter Filter[MaximumWindows][MaximumFilters];
} CTnsData;


#ifndef __PULSE_DATA__
#define __PULSE_DATA__
enum
{
	MaximumLines = 4
};
typedef struct
{
	char PulseDataPresent;
	char NumberPulse;
	char PulseStartBand;
	char PulseOffset[MaximumLines];
	char PulseAmp[MaximumLines];
} CPulseData;
#endif 

typedef struct
{
	char CommonWindow;
	char ElementInstanceTag;
	unsigned char GlobalGain;
} CRawDataInfo;

typedef struct {
	unsigned char pnsUsed[PNS_BAND_FLAGS_SIZE];
	int CurrentEnergy;
	unsigned char PnsActive;
} CPnsData;

typedef struct {
	unsigned char correlated[PNS_BAND_FLAGS_SIZE];
} CPnsInterChannelData;


typedef struct
{
	short *pScaleFactor;
	char  *pCodeBook;

	float *pSpectralCoefficient;

	CIcsInfo IcsInfo;
	CTnsData TnsData;
	CPulseData PulseData;
	CRawDataInfo RawDataInfo;
	CJointStereoData *pJointStereoData;

	CPnsData PnsData;
	CPnsInterChannelData PnsInterChannelData;
} CAacDecoderChannelInfo;

#endif //__CHANNEL_INFO_H__

