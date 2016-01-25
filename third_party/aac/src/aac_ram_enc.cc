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

 $Id: aac_ram.c,v 1.1.2.15 2004/05/30 13:57:40 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief memory requirement in dynamic and static RAM  $Revision: 1.1.2.15 $
  \author Markus Werner
*/

#include <stdio.h>
#include "aac_ram_enc.h"
#include "aacenc.h"
#include "psy_const.h"
#include "FloatFR.h"
#include "sbr_ram_enc.h"

/*
  Static memory areas, must not be overwritten in other sections of the encoder
*/

/* aac Encoder mdct delay buffer */
//float mdctDelayBuffer_2[MAX_CHANNELS*BLOCK_SWITCHING_OFFSET];

/*
  these tables are initialized once at application start
  and are not touched thereafter
*/

//int sideInfoTabLong[MAX_SFB_LONG + 1];
//int sideInfoTabShort[MAX_SFB_SHORT + 1];

/*
  Dynamic memory areas, might be reused in other algorithm sections,
*/


/* quantized spectrum */ 
//short *quantSpec = (short*)PsBuf3;

/* scratch space for quantization */
//float *expSpec = sbr_envIBuffer; /* FRAME_LEN_LONG values */
//short *quantSpecTmp = (short*) &sbr_envIBuffer[FRAME_LEN_LONG];

/* scalefactors */ 
//short *scf_2= (short*) &sbr_envIBuffer[2*FRAME_LEN_LONG]; /*[MAX_CHANNELS*MAX_GROUPED_SFB];*/

/* max spectral values pre sfb */
//unsigned short *maxValueInSfb = (unsigned short*) &sbr_envIBuffer[2*FRAME_LEN_LONG+MAX_CHANNELS*MAX_GROUPED_SFB]; /* [MAX_CHANNELS*MAX_GROUPED_SFB]; */

