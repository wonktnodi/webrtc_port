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

 $Id: freq_sca.h,v 1.1.1.1.2.1 2004/02/17 10:21:22 jr Exp $

*******************************************************************************/
/*!
  \file
  \brief  frequency scale prototypes $Revision: 1.1.1.1.2.1 $
  \author Kristofer Kjoerling & Per Ekstrand
*/
#ifndef __FREQ_SCA2_H
#define __FREQ_SCA2_H
#include "sbr_def.h"

#define MAX_OCTAVE        29
#define MAX_SECOND_REGION 50


int
UpdateFreqScale(unsigned char *v_k_master, int *h_num_bands,
                const int k0, const int k2,
                const int freq_scale,
                const int alter_scale);

int
UpdateHiRes(unsigned char *h_hires,
            int *num_hires,
            unsigned char *v_k_master,
            int num_master ,
            int *xover_band,
            SR_MODE drOrSr,
            int noQMFChannels);

void  UpdateLoRes(unsigned char * v_lores,
                  int *num_lores,
                  unsigned char * v_hires,
                  int num_hires);

int
FindStartAndStopBand(const int samplingFreq,
                     const int noChannels,
                     const int startFreq,
                     const int stop_freq,
                     const SR_MODE sampleRateMode,
                     int *k0,
                     int *k2);

int getSbrStartFreqRAW (int startFreq, int QMFbands, int fs );
int getSbrStopFreqRAW  (int stopFreq, int QMFbands, int fs);
#endif
