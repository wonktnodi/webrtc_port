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

 $Id: streaminfo.h,v 1.1.1.1.6.2 2004/02/16 15:52:53 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  current bitstream parameters  $Revision: 1.1.1.1.6.2 $
  \author zerok 
*/

#ifndef STREAMINFO_H
#define STREAMINFO_H

typedef struct
{
  char SamplingRateIndex;
  unsigned int SamplingRate;
  unsigned int Profile;
  unsigned int ChannelConfig;
  unsigned int Channels;
  unsigned int BitRate;
  unsigned int SamplesPerFrame;
} CStreamInfo;

/* Initialize stream info */
void CStreamInfoOpen(CStreamInfo&StreamInfo);

#endif /* #ifndef STREAMINFO_H */
