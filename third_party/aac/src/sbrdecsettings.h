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

 $Id: sbrdecsettings.h,v 1.1.1.1.6.2 2004/02/17 15:36:17 jr Exp $

*******************************************************************************/
/*!
  \file
  \brief  Default Decoder Settings $Revision: 1.1.1.1.6.2 $
  \author Thomas Ziegler
*/
#ifndef _SBRDECSETTINGS_H
#define _SBRDECSETTINGS_H

#define SBR_UPSAMPLE_FAC        2


#ifdef MAX_FRAME_SIZE
#undef MAX_FRAME_SIZE
#endif
#define MAX_FRAME_SIZE       1024


#define NO_ACTUAL_SYNTHESIS_CHANNELS  64
#define QMF_FILTER_STATE_SYN_SIZE     640
#define QMF_FILTER_STATE_ANA_SIZE     320
#define NO_SYNTHESIS_CHANNELS         64
#define NO_ANALYSIS_CHANNELS   (NO_SYNTHESIS_CHANNELS / SBR_UPSAMPLE_FAC)
#define NO_POLY                (QMF_FILTER_STATE_SYN_SIZE / (2*NO_SYNTHESIS_CHANNELS))

#define NO_SYNTHESIS_CHANNELS_DOWN_SAMPLED     32
#define QMF_FILTER_STATE_SYN_SIZE_DOWN_SAMPLED 320


#define MAX_NOISE_ENVELOPES      2
#define MAX_NOISE_COEFFS         5
#define MAX_NUM_NOISE_VALUES     (MAX_NOISE_ENVELOPES * MAX_NOISE_COEFFS)
#define MAX_NUM_LIMITERS        12

#ifdef MAX_ENVELOPES
#undef MAX_ENVELOPES
#endif
#define MAX_ENVELOPES           5

#define MAX_FREQ_COEFFS         48
#define MAX_FREQ_COEFFS_FS44100 35
#define MAX_FREQ_COEFFS_FS48000 32


#define MAX_NUM_ENVELOPE_VALUES (MAX_ENVELOPES * MAX_FREQ_COEFFS)

#define MAX_GAIN_EXP            34

#define LPC_ORDER       2

#define MAX_INVF_BANDS          MAX_NOISE_COEFFS

#define MAX_COLS               (MAX_FRAME_SIZE / NO_ANALYSIS_CHANNELS)
#define MAX_OV_COLS            6
#define MAX_ENV_COLS           (MAX_COLS + MAX_OV_COLS)


#endif
