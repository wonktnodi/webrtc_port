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

$Id: invf_est.h,v 1.2.2.2 2004/02/17 16:12:58 kk Exp $

*******************************************************************************/
/*!
\file
\brief  Inverse Filtering detection prototypes $Revision: 1.2.2.2 $
\author <fill in initial author>
*/
#ifndef _INV_FILT_DET_H
#define _INV_FILT_DET_H

#include "sbr_main.h"
#include "sbr_def.h"


#define INVF_SMOOTHING_LENGTH 2

typedef struct __tagDETECTOR_PARAMETERS
{
	const float *quantStepsSbr;
	const float *quantStepsOrig;
	const float *nrgBorders;
	int   numRegionsSbr;
	int   numRegionsOrig;
	int   numRegionsNrg;
	const INVF_MODE regionSpace[5][5];
	const INVF_MODE regionSpaceTransient[5][5];
	const int EnergyCompFactor[5];
}DETECTOR_PARAMETERS;



typedef struct
{
	float  origQuotaMean[INVF_SMOOTHING_LENGTH+1];
	float  sbrQuotaMean[INVF_SMOOTHING_LENGTH+1];
	float origQuotaMeanFilt;
	float sbrQuotaMeanFilt;
	float avgNrg;

}DETECTOR_VALUES;



typedef struct
{

	int prevRegionSbr[MAX_NUM_NOISE_VALUES];
	int prevRegionOrig[MAX_NUM_NOISE_VALUES];

	float nrgAvg;

	int freqBandTableInvFilt[MAX_NUM_NOISE_VALUES];
	int noDetectorBands;
	int noDetectorBandsMax;

	const DETECTOR_PARAMETERS *detectorParams;
	INVF_MODE prevInvfMode[MAX_NUM_NOISE_VALUES];
	DETECTOR_VALUES detectorValues[MAX_NUM_NOISE_VALUES];


	float wmQmf[MAX_NUM_NOISE_VALUES];
}
SBR_INV_FILT_EST;

typedef SBR_INV_FILT_EST *HANDLE_SBR_INV_FILT_EST;


void
qmfInverseFilteringDetector (HANDLE_SBR_INV_FILT_EST hInvFilt,
							 float ** quotaMatrix,
							 float *nrgVector,
							 char * indexVector,
							 int startIndex,
							 int stopIndex,
							 int transientFlag,
							 INVF_MODE* infVec);



int
createInvFiltDetector (HANDLE_SBR_INV_FILT_EST hInvFilt,
					   int* freqBandTableDetector,
					   int numDetectorBands,
					   int numberOfEstimatesPerFrame,
					   unsigned int useSpeechConfig);

void
deleteInvFiltDetector (HANDLE_SBR_INV_FILT_EST hInvFilt);


int
resetInvFiltDetector(HANDLE_SBR_INV_FILT_EST hInvFilt,
					 int* freqBandTableDetector,
					 int numDetectorBands);



#endif /* _QMF_INV_FILT_H */

