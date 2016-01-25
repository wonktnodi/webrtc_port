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
 
 $Id: bitenc.h,v 1.1.1.1.2.4 2004/02/16 20:02:18 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  Bitstream encoder $Revision: 1.1.1.1.2.4 $
  \author M. Werner
*/
#ifndef _BITENC_H
#define _BITENC_H

#include "qc_data.h"
#include "tns_enc.h"
#include "channel_map.h"
#include "interface.h"

struct BITSTREAMENCODER_INIT
{
  int nChannels;
  int bitrate;
  int sampleRate;
  int profile;
};



int WriteBitstream (HANDLE_BIT_BUF hBitstream,
                    ELEMENT_INFO elInfo,
                    QC_OUT* qcOut,
                    PSY_OUT* psyOut,
                    int* globUsedBits,
                    const unsigned char *ancBytes
                    );

#endif /* _BITENC_H */
