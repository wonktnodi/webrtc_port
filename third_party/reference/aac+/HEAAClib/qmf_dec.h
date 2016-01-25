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

 $Id: qmf_dec.h,v 1.11.6.6 2004/04/01 07:31:14 ehr Exp $

******************************************************************************/
/*
\file
\brief Complex qmf analysis/synthesis
$Revision: 1.11.6.6 $
*/
#ifndef __QMF_DEC_H
#define __QMF_DEC_H

#include "sbrdecsettings.h"
#include "ps_dec.h"
#include <string>


struct dct4Twiddle
{
  const float *cos_twiddle;
  const float *sin_twiddle;
  const float *alt_sin_twiddle;
};

typedef struct __tagSBR_QMF_FILTER_BANK_DEC
{
  int no_channels;

  const float *p_filter;

  struct dct4Twiddle *pDct4Twiddle;

#ifndef LP_SBR_ONLY

  const float *cos_twiddle;
  const float *sin_twiddle;
  const float *alt_sin_twiddle;

  const float *t_cos;
  const float *t_sin;

#endif

  float *FilterStatesAna;
  float *FilterStatesSyn;

  int no_col;

  int lsb;
  int usb;

  int qmf_filter_state_size;

  float sbr_QmfStatesSynthesis[MAXNRQMFCHANNELS * QMF_FILTER_STATE_SYN_SIZE];
  __tagSBR_QMF_FILTER_BANK_DEC(){
    memset(this, 0, sizeof(__tagSBR_QMF_FILTER_BANK_DEC));
  }
}
SBR_QMF_FILTER_BANK_DEC;

typedef SBR_QMF_FILTER_BANK_DEC *HANDLE_SBR_QMF_FILTER_BANK_DEC;

void
cplxAnalysisQmfFiltering (
                          const float *timeIn,
                          float **qmfReal,
#ifndef LP_SBR_ONLY
                          float **qmfImag,
#endif
                          HANDLE_SBR_QMF_FILTER_BANK_DEC qmfBank,
                          int   bUseLP);

void
cplxSynthesisQmfFiltering (float **qmfReal,
#ifndef LP_SBR_ONLY
                           float **qmfImag,
#endif
                           float *timeOut,
                           HANDLE_SBR_QMF_FILTER_BANK_DEC qmfBank,
                           int   bUseLP,
                           HANDLE_PS_DEC ps_d,
                           int   active
                           );


int
createCplxAnalysisQmfBank (HANDLE_SBR_QMF_FILTER_BANK_DEC h_sbrQmf,
                           HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
						   int noCols,
                           int lsb,
                           int usb,
                           int chan);

int
createCplxSynthesisQmfBank (HANDLE_SBR_QMF_FILTER_BANK_DEC h_sbrQmf,
                            int noCols,
                            int lsb,
                            int usb,
                            int chan,
                            int bDownSample);
#endif
