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
 
 $Id: sf_estim.h,v 1.2.2.3 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Scalefactor estimation $Revision: 1.2.2.3 $
  \author M. Werner
*/
#ifndef __SF_ESTIM_H__
#define __SF_ESTIM_H__

#include "psy_const.h"
#include "interface.h"
#include "qc_data.h"

void
CalcFormFactor(float sfbFormFactor[MAX_CHANNELS][MAX_GROUPED_SFB],
               float sfbNRelevantLines[MAX_CHANNELS][MAX_GROUPED_SFB],
               PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS],
               const int nChannels);

void
EstimateScaleFactors(PSY_OUT_CHANNEL psyOutChannel[MAX_CHANNELS],
                     QC_OUT_CHANNEL   qcOutChannel[MAX_CHANNELS],
                     float sfbFormFactor[MAX_CHANNELS][MAX_GROUPED_SFB],
                     float sfbNRelevantLines[MAX_CHANNELS][MAX_GROUPED_SFB],
                     const int nChannels);

#endif
