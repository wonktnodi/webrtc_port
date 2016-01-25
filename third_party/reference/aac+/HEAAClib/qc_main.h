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
 
 $Id: qc_main.h,v 1.2.2.4 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Quantizing & coding main module $Revision: 1.2.2.4 $
  \author M. Werner
*/
#ifndef _QC_MAIN_H
#define _QC_MAIN_H

#include "qc_data.h"
#include "interface.h"


int  QCOutNew(QC_OUT *hQC, int nChannels);

void QCOutDelete(QC_OUT *hQC);

int  QCNew(QC_STATE *hQC);

int  QCInit(QC_STATE *hQC, struct QC_INIT *init);
void QCDelete(QC_STATE *hQC);


int QCMain(QC_STATE *hQC,
           int nChannels,
           ELEMENT_BITS* elBits,
           ATS_ELEMENT* adjThrStateElement,
           PSY_OUT_CHANNEL psyOutChannel[MAX_CHANNELS],
           PSY_OUT_ELEMENT* psyOutElement,
           QC_OUT_CHANNEL  qcOutChannel[MAX_CHANNELS],
           QC_OUT_ELEMENT* qcOutElement,
           int ancillaryDataBytes);

void UpdateBitres(QC_STATE* qcKernel,
                  QC_OUT* qcOut);

int FinalizeBitConsumption(QC_STATE *hQC,
                           QC_OUT* qcOut);

int AdjustBitrate(QC_STATE *hQC,
                  int bitRate,
                  int sampleRate);

#endif /* _QC_MAIN_H */
