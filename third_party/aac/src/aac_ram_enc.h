/****************************************************************************

                      (C) copyright Coding Technologies (2004)
                               All Rights Reserved

 This software module was developed by Coding Technologies (CT). This is
 company confidential information and the property of CT, and can not be
 reproduced or disclosed in any form without written authorization of CT.

 Those intending to use this software module for other purposes are advised
 that this infringe existing or pending patents. CT has no liability for
 use of this software module or derivatives thereof in any implementation.
 Copyright is not released for any means. CT retains full right to use the
 code for its own purpose, assign or sell the code to a third party and to
 inhibit any user or third party from using the code. This copyright notice
 must be included in all copies or derivative works.

 $Id: aac_ram.h,v 1.3.2.12 2004/05/30 14:15:18 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief memory requirement in dynamic and static RAM  $Revision: 1.3.2.12 $
  \author Markus Werner
*/

#ifndef AAC_RAM_H
#define AAC_RAM_H

#include "psy_const.h"

//extern float mdctDelayBuffer_2[MAX_CHANNELS*BLOCK_SWITCHING_OFFSET];

// extern int sideInfoTabLong[MAX_SFB_LONG + 1];
// extern int sideInfoTabShort[MAX_SFB_SHORT + 1];

//extern short *quantSpec_2;
//extern float *expSpec;
//extern short *quantSpecTmp_2;

//extern short *scf_2;
//extern unsigned short *maxValueInSfb_2;

/* dynamic buffers of SBR that can be reused by the core */

//extern float sbr_envRBuffer[];
//extern float sbr_envIBuffer[];

#endif
