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

 $Id: sbr_ram.h,v 1.11.6.5 2004/02/18 13:14:22 ehr Exp $

*******************************************************************************/
/*
\file
\brief Memory layout
$Revision: 1.11.6.5 $
*/
#ifndef __SBR_RAM_H
#define __SBR_RAM_H

#include "sbrdecsettings.h" /* for MAXNRSBRCHANNELS */
#include "sbrdecoder.h"
#include "env_extr.h"

//extern  float  sbr_CodecQmfStatesAnalysis[];
//extern  float  sbr_QmfStatesSynthesis[];

//extern  float sbr_GainSmooth[MAXNRSBRCHANNELS][MAX_FREQ_COEFFS];
//extern  float sbr_NoiseSmooth[MAXNRSBRCHANNELS][MAX_FREQ_COEFFS];
//extern  char   sbr_GainSmooth_e[MAXNRSBRCHANNELS][MAX_FREQ_COEFFS];

//extern FREQ_BAND_DATA FreqBandData;

#ifdef LP_SBR_ONLY
//extern float   sbr_OverlapBuffer[MAXNRQMFCHANNELS][MAX_OV_COLS * NO_SYNTHESIS_CHANNELS];
#else
//extern float   sbr_OverlapBuffer[MAXNRQMFCHANNELS][2 * MAX_OV_COLS * NO_SYNTHESIS_CHANNELS];
#endif

//extern float   sbr_LpcFilterStatesReal[MAXNRQMFCHANNELS][LPC_ORDER][NO_ANALYSIS_CHANNELS];
#ifndef LP_SBR_ONLY
//extern float   sbr_LpcFilterStatesImag[MAXNRQMFCHANNELS][LPC_ORDER][NO_ANALYSIS_CHANNELS];
#endif

//extern TRANSPOSER_SETTINGS sbr_TransposerSettings;

//extern SBR_PREV_FRAME_DATA PrevFrameData[MAXNRSBRCHANNELS];

//extern SBRBITSTREAM sbr_PrevBitstream;


#ifdef LP_SBR_ONLY
extern float   *InterimResult;
#else
//extern float   *WorkBuffer2;
//extern float   InterimResult[MAX_FRAME_SIZE];
#endif

#endif

