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

 $Id: env_dec.h,v 1.1.1.1.8.1 2004/02/17 15:36:16 jr Exp $

*******************************************************************************/
/*
  \file
  \brief  Envelope decoding $Revision: 1.1.1.1.8.1 $
  \author Per Ekstrand
*/
#ifndef __ENV_DEC_H
#define __ENV_DEC_H

#include "env_extr.h"

void decodeSbrData (HANDLE_SBR_HEADER_DATA_DEC hHeaderData,
                    HANDLE_SBR_FRAME_DATA h_data_left,
		    HANDLE_SBR_PREV_FRAME_DATA h_prev_data_left,
		    HANDLE_SBR_FRAME_DATA h_data_right,
		    HANDLE_SBR_PREV_FRAME_DATA h_prev_data_right);


#endif
