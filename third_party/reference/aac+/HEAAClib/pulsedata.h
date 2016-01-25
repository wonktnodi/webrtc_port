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

 $Id: pulsedata.h,v 1.1.1.1.6.2 2004/02/16 15:52:53 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  pulse data tool  $Revision: 1.1.1.1.6.2 $
  \author zerok 
*/

#ifndef PULSEDATA_H
#define PULSEDATA_H

#include "bitstream.h"
#include "FFR_bitbuffer.h"


#ifndef __PULSE_DATA__
#define __PULSE_DATA__
enum
{
	MaximumLines = 4
};
typedef struct
{
  char PulseDataPresent;
  char NumberPulse;
  char PulseStartBand;
  char PulseOffset[MaximumLines];
  char PulseAmp[MaximumLines];
} CPulseData;
#endif 
void CPulseData_Read(HANDLE_BIT_BUF bs,
                     CPulseData *PulseData);

void CPulseData_Apply(CPulseData *PulseData,
                      const short *pScaleFactorBandOffsets,
                      int *coef);

#endif /* #ifndef PULSEDATA_H */
