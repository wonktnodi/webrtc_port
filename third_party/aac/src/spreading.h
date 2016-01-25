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
 
 $Id: spreading.h,v 1.2.2.1 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Spreading of energy and weighted tonality $Revision: 1.2.2.1 $
  \author M. Werner
*/
#ifndef _SPREADING_H
#define _SPREADING_H

void SpreadingMax(const int    pbCnt,
                  const float *maskLowFactor,
                  const float *maskHighFactor,
                  float       *pbSpreadedEnergy);

#endif /* #ifndef _SPREADING_H */
