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

 $Id: streaminfo.c,v 1.1.2.3 2004/02/16 15:52:53 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  current bitstream parameters  $Revision: 1.1.2.3 $
  \author zerok 
*/

#include "aac_ram_dec.h"
#include "streaminfo.h"

#include "flc.h" /* the 3GPP instrumenting tool */

/*  Stream Configuration and Information.

    This class holds configuration and information data for a stream to be decoded. It
    provides the calling application as well as the decoder with substantial information,
    e.g. profile, sampling rate, number of channels found in the bitstream etc.
*/
void CStreamInfoOpen(CStreamInfo &streamInfo)
{
  FLC_sub_start("CStreamInfoOpen");

  /* initialize CStreamInfo */
  PTR_INIT(1); MOVE(1);
  //pStreamInfo[0] = &StreamInfo;

  MOVE(7);
  streamInfo.SamplingRateIndex = 0;
  streamInfo.SamplingRate = 0;
  streamInfo.Profile = 0;
  streamInfo.ChannelConfig = 0;
  streamInfo.Channels = 0;
  streamInfo.BitRate = 0;
  streamInfo.SamplesPerFrame = FRAME_SIZE;

  FLC_sub_end();
}


