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

 $Id: aac_rom.h,v 1.1.2.9 2004/05/30 14:15:18 ehr Exp $

*******************************************************************************/
/*
  \file
  \brief Declaration of constant tables
  $Revision: 1.1.2.9 $
*/
#ifndef AAC_ROM_H
#define AAC_ROM_H

#include "psy_const.h"
#include "tns_param.h"

#define LD_FFT_TWIDDLE_TABLE_SIZE 9
#define FFT_TWIDDLE_TABLE_SIZE (1 << LD_FFT_TWIDDLE_TABLE_SIZE)


/*
  mdct
*/
extern const float LongWindowSine [FRAME_LEN_LONG];
extern const float ShortWindowSine [FRAME_LEN_SHORT];
extern const float LongWindowKBD [FRAME_LEN_LONG];
extern const float fftTwiddleTab[FFT_TWIDDLE_TABLE_SIZE+1];



/*
  quantizer
*/
extern const float quantTableQ[16];
extern const float quantTableE[17];
extern const float invQuantTableQ[16];
extern const float invQuantTableE[17];
#define MAX_POW4_3_TABLE 64
extern const float pow4_3_tab[MAX_POW4_3_TABLE];



/*
  huffman
*/
extern const unsigned short huff_ltab1_2[3][3][3][3];
extern const unsigned short huff_ltab3_4[3][3][3][3];
extern const unsigned short huff_ltab5_6[9][9];
extern const unsigned short huff_ltab7_8[8][8];
extern const unsigned short huff_ltab9_10[13][13];
extern const unsigned short huff_ltab11[17][17];
extern const unsigned short huff_ltabscf[121];
extern const unsigned short huff_ctab1[3][3][3][3];
extern const unsigned short huff_ctab2[3][3][3][3];
extern const unsigned short huff_ctab3[3][3][3][3];
extern const unsigned short huff_ctab4[3][3][3][3];
extern const unsigned short huff_ctab5[9][9];
extern const unsigned short huff_ctab6[9][9];
extern const unsigned short huff_ctab7[8][8];
extern const unsigned short huff_ctab8[8][8];
extern const unsigned short huff_ctab9[13][13];
extern const unsigned short huff_ctab10[13][13];
extern const unsigned short huff_ctab11[17][17];
extern const unsigned long  huff_ctabscf[121];



/*
  misc
*/

extern const unsigned char sfb_11025_long_1024[];
extern const unsigned char sfb_11025_short_128[];
extern const unsigned char sfb_12000_long_1024[];
extern const unsigned char sfb_12000_short_128[];
extern const unsigned char sfb_16000_long_1024[];
extern const unsigned char sfb_16000_short_128[];
extern const unsigned char sfb_22050_long_1024[];
extern const unsigned char sfb_22050_short_128[];
extern const unsigned char sfb_24000_long_1024[];
extern const unsigned char sfb_24000_short_128[];

/*
  TNS
 */

extern const TNS_MAX_TAB_ENTRY tnsMaxBandsTab[9];
extern const TNS_CONFIG_TABULATED  p_8000_mono_long;
extern const TNS_CONFIG_TABULATED  p_8000_mono_short;
extern const TNS_CONFIG_TABULATED  p_8000_stereo_long;
extern const TNS_CONFIG_TABULATED  p_8000_stereo_short;
extern const TNS_CONFIG_TABULATED  p_16000_mono_long;
extern const TNS_CONFIG_TABULATED  p_16000_mono_short;
extern const TNS_CONFIG_TABULATED  p_16000_stereo_long;
extern const TNS_CONFIG_TABULATED  p_16000_stereo_short;
extern const TNS_CONFIG_TABULATED  p_24000_mono_long;
extern const TNS_CONFIG_TABULATED  p_24000_mono_short;
extern const TNS_CONFIG_TABULATED  p_24000_stereo_long;
extern const TNS_CONFIG_TABULATED  p_24000_stereo_short;
extern const TNS_CONFIG_TABULATED  p_32000_mono_long;
extern const TNS_CONFIG_TABULATED  p_32000_mono_short;
extern const TNS_CONFIG_TABULATED  p_32000_stereo_long;
extern const TNS_CONFIG_TABULATED  p_32000_stereo_short;
extern const TNS_INFO_TAB tnsInfoTab[4];

extern const float tnsCoeff3[8];
extern const float tnsCoeff3Borders[8];
extern const float tnsCoeff4[16];
extern const float tnsCoeff4Borders[16];

#endif
