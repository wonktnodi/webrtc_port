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

 $Id: bit_sbr.h,v 1.3.2.4 2004/04/01 13:14:16 ehr Exp $

*******************************************************************************/
/*!
  \file
  \brief  SBR bit writing $Revision: 1.3.2.4 $
  \author Holger Hoerich,
*/
#ifndef __BIT_SBR_H
#define __BIT_SBR_H


struct SBR_HEADER_DATA_ENC;
struct SBR_BITSTREAM_DATA;
struct SBR_ENV_DATA;
struct COMMON_DATA;
struct PS_ENC;


int WriteEnvSingleChannelElement(struct SBR_HEADER_DATA_ENC    *sbrHeaderData,
                                 struct SBR_BITSTREAM_DATA *sbrBitstreamData,
                                 struct SBR_ENV_DATA       *sbrEnvData,
                                 struct PS_ENC             *h_ps_e,
                                 struct COMMON_DATA        *cmonData);


int WriteEnvChannelPairElement(struct SBR_HEADER_DATA_ENC    *sbrHeaderData,
                               struct SBR_BITSTREAM_DATA *sbrBitstreamData,
                               struct SBR_ENV_DATA       *sbrEnvDataLeft,
                               struct SBR_ENV_DATA       *sbrEnvDataRight,
                               struct COMMON_DATA        *cmonData);



int CountSbrChannelPairElement (struct SBR_HEADER_DATA_ENC     *sbrHeaderData,
                                struct SBR_BITSTREAM_DATA  *sbrBitstreamData,
                                struct SBR_ENV_DATA        *sbrEnvDataLeft,
                                struct SBR_ENV_DATA        *sbrEnvDataRight,
                                struct COMMON_DATA         *cmonData);




#endif
