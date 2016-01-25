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

 $Id: FloatFR.h,v 1.5.2.7 2004/05/30 13:59:40 ehr Exp $

*******************************************************************************/
/*!
  \file
  \brief  Floating Point Reference Library $Revision: 1.5.2.7 $
  \author Klaus Peichl / Andreas Ehret
*/

#ifndef __FLOATFR_H
#define __FLOATFR_H

#include <stdarg.h>


#define LOG_DUALIS_TABLE_SIZE 65

#define INV_TABLE_BITS 8
#define INV_TABLE_SIZE (1<<INV_TABLE_BITS)

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif


void   FloatFR_Init ( void );
float  FloatFR_logDualis(int a);
float  FloatFR_getNumOctaves(int a, int b);
#endif
