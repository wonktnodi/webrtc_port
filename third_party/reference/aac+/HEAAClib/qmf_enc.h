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

 $Id: qmf_enc.h,v 1.8.2.4 2004/04/01 13:14:17 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  QMF analysis structs and prototypes $Revision: 1.8.2.4 $
  \author Kristofer Kjoerling & Per Ekstrand
*/
#ifndef __QMF_ENC_H
#define __QMF_ENC_H
#include "aacenc.h"
#include "sbr_def.h"
#include <string>
typedef struct __tagSBR_QMF_FILTER_BANK_ENC
{
  const float *p_filter;

  const float *cos_twiddle;
  const float *sin_twiddle;
  const float *alt_sin_twiddle;

  float *timeBuffer;
  float *workBuffer;

  float *qmf_states_buffer;

  float sbr_QmfStatesAnalysis[MAX_CHANNELS  * QMF_FILTER_LENGTH];

  float PsBuf5[1024*4];
  __tagSBR_QMF_FILTER_BANK_ENC()
  {
	  memset(sbr_QmfStatesAnalysis, 0, sizeof(sbr_QmfStatesAnalysis));
  }
}
SBR_QMF_FILTER_BANK_ENC;

typedef SBR_QMF_FILTER_BANK_ENC *HANDLE_SBR_QMF_FILTER_BANK_ENC;


void sbrAnalysisFiltering (const float *timeIn,
                           int   timeInStride,
                           float **rAnalysis,
                           float **iAnalysis,
                           HANDLE_SBR_QMF_FILTER_BANK_ENC qmfBank);


int createQmfBank (int chan,HANDLE_SBR_QMF_FILTER_BANK_ENC h_sbrQmf);


void deleteQmfBank (HANDLE_SBR_QMF_FILTER_BANK_ENC  h_sbrQmf);


void
getEnergyFromCplxQmfData (float **energyValues,
                          float **realValues,
                          float **imagValues

                          );

void
SynthesisQmfFiltering (float **sbrReal,
                       float **sbrImag,
                       float *timeOut,
                       HANDLE_SBR_QMF_FILTER_BANK_ENC qmfBank);


int
CreateSynthesisQmfBank (HANDLE_SBR_QMF_FILTER_BANK_ENC h_sbrQmf);




void DeleteSynthesisQmfBank (HANDLE_SBR_QMF_FILTER_BANK_ENC *h_sbrQmf);

#endif
