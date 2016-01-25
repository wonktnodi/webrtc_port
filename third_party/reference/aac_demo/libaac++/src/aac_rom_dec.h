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

 $Id: aac_rom.h,v 1.6.6.8 2004/05/30 14:11:13 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  Definition of constant tables  $Revision: 1.6.6.8 $
  \author zerok 
*/

#ifndef AAC_ROM_H
#define AAC_ROM_H

#include "block.h"

#define AAC_NF_NO_RANDOM_VAL  512   /*!< Size of random number array for noise floor */

extern const float OnlyLongWindowSine[FRAME_SIZE];
extern const float OnlyShortWindowSine[FRAME_SIZE/8];
extern const float OnlyLongWindowKBD[FRAME_SIZE];
extern const float OnlyShortWindowKBD[FRAME_SIZE/8];


extern const char specExpTableComb[4][14];

extern const float trigData[513];

extern const SamplingRateInfo SamplingRateInfoTable[9];

extern const CodeBookDescription HuffmanCodeBooks[13];

extern const char tns_max_bands_tbl[9][2];

extern const float tnsCoeff3[8];

extern const float tnsCoeff4[16];

#endif /* #ifndef AAC_ROM_H */
