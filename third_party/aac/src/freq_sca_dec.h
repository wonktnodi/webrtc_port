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

  $Id: freq_sca.h,v 1.1.1.1.8.1 2004/02/17 15:36:17 jr Exp $
****************************************************************************/
/*
  \file
  \brief   Frequency scale prototypes $Revision: 1.1.1.1.8.1 $
*/
#ifndef __FREQ_SCA_H
#define __FREQ_SCA_H

#include "env_extr.h"

int
sbrdecUpdateFreqScale(unsigned char * v_k_master,
                      unsigned char *numMaster,
                      SBR_HEADER_DATA_DEC * headerData);

void sbrdecUpdateHiRes(unsigned char * v_hires, unsigned char *num_hires, unsigned char * v_k_master, unsigned char num_bands, unsigned char xover_band);
void sbrdecUpdateLoRes(unsigned char * v_lores, unsigned char *num_lores, unsigned char * v_hires, unsigned char num_hires);


void sbrdecDownSampleLoRes(unsigned char *v_result, unsigned char num_result,
                           unsigned char *freqBandTableRef, unsigned char num_Ref);

int sbrdecGetSbrStartFreqRAW (int startFreq, int QMFbands, int fs );
int sbrdecGetSbrStopFreqRAW  (int stopFreq, int QMFbands, int fs);

void shellsort(unsigned char *in, unsigned char n);

int
resetFreqBandTables(HANDLE_SBR_HEADER_DATA_DEC hHeaderData);

#endif
