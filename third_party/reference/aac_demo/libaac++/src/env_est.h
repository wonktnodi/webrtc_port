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

 $Id: env_est.h,v 1.7.2.9 2004/05/31 12:50:25 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  Envelope estimation structs and prototypes $Revision: 1.7.2.9 $
  \author Kristofer Kjoerling & Per Ekstrand
*/
#ifndef __ENV_EST_H
#define __ENV_EST_H

#include "sbr_def.h"
#include "aacenc.h"
#include "qmf_enc.h"

typedef struct
{
  int pre_transient_info[2];
  float *rBuffer[QMF_TIME_SLOTS];


  float  sbr_envIBuffer [MAX_CHANNELS * QMF_TIME_SLOTS * QMF_CHANNELS];
  float *iBuffer[QMF_TIME_SLOTS];
  float *YBuffer[QMF_TIME_SLOTS*2];

  char envelopeCompensation[MAX_FREQ_COEFFS];

  int YBufferWriteOffset;

  int no_cols;
  int no_rows;
  int start_index;

  int time_slots;
  int time_step;

  float  sbr_envRBuffer [MAX_CHANNELS * QMF_TIME_SLOTS * QMF_CHANNELS];
}
SBR_EXTRACT_ENVELOPE;
typedef SBR_EXTRACT_ENVELOPE *HANDLE_SBR_EXTRACT_ENVELOPE;


/************  Function Declarations ***************/

int
CreateExtractSbrEnvelope (int chan,
                          HANDLE_SBR_EXTRACT_ENVELOPE hSbr,
                          int start_index,
						  HANDLE_SBR_ENCODER hEnvEncoder
                          );

void deleteExtractSbrEnvelope (HANDLE_SBR_EXTRACT_ENVELOPE hSbrCut);



struct SBR_CONFIG_DATA_DEC;
struct SBR_HEADER_DATA_DEC;
struct SBR_BITSTREAM_DATA;
struct ENV_CHANNEL;
struct COMMON_DATA;
struct PS_ENC;


void
extractSbrEnvelope(float *timeInPtr,
                   float *pCoreBuffer,
                   unsigned int timeInStride,
                   struct SBR_CONFIG_DATA *h_con,
                   struct SBR_HEADER_DATA_ENC  *sbrHeaderData,
                   struct SBR_BITSTREAM_DATA *sbrBitstreamData,
                   struct ENV_CHANNEL      *h_envChan[MAX_CHANNELS],
                   struct PS_ENC *hPsEnc,
                   HANDLE_SBR_QMF_FILTER_BANK_ENC hSynthesisQmfBank,
                   struct COMMON_DATA      *cmonData);

#endif
