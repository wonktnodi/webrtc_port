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
 
 $Id: psy_const.h,v 1.3.2.6 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Global psychoaccoustic constants $Revision: 1.3.2.6 $
  \author M. Werner
*/
#ifndef _PSYCONST_H
#define _PSYCONST_H

#include "aacenc.h"

#define TRUE  1
#define FALSE 0

#define FRAME_LEN_LONG    AACENC_BLOCKSIZE
#define TRANS_FAC         8
#define FRAME_LEN_SHORT   (FRAME_LEN_LONG/TRANS_FAC)

/* Block types */
enum
{
  LONG_WINDOW = 0,
  START_WINDOW,
  SHORT_WINDOW,
  STOP_WINDOW
};

/* Window shapes */
enum
{
  SINE_WINDOW = 0,
  KBD_WINDOW  = 1
};

/*
  MS stuff
*/
enum
{
  SI_MS_MASK_NONE = 0,
  SI_MS_MASK_SOME = 1,
  SI_MS_MASK_ALL  = 2
};


#define MAX_NO_OF_GROUPS 4

#define MAX_SFB_SHORT   15
#define MAX_SFB_LONG    51
#define MAX_SFB         (MAX_SFB_SHORT > MAX_SFB_LONG ? MAX_SFB_SHORT : MAX_SFB_LONG)   /* = MAX_SFB_LONG */
#define MAX_GROUPED_SFB (MAX_NO_OF_GROUPS*MAX_SFB_SHORT > MAX_SFB_LONG ? \
                         MAX_NO_OF_GROUPS*MAX_SFB_SHORT : MAX_SFB_LONG)

#define BLOCK_SWITCHING_OFFSET		   (1*1024+3*128+64+128)
#define BLOCK_SWITCHING_DATA_SIZE          FRAME_LEN_LONG
										    
#define MAX_CHANNEL_BITS         6144

#define TRANSFORM_OFFSET_LONG    0
#define TRANSFORM_OFFSET_SHORT   448


#define PCM_LEVEL             1.0f
#define NORM_PCM              (PCM_LEVEL)
#define NORM_PCM_ENERGY       (NORM_PCM*NORM_PCM)



#endif /* _PSYCONST_H */
