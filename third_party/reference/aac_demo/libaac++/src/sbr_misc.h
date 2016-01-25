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

 $Id: sbr_misc.h,v 1.2.2.2 2004/02/17 16:12:58 kk Exp $

*******************************************************************************/
/*!
  \file
  \brief  Sbr miscellaneous helper functions prototypes $Revision: 1.2.2.2 $
  \author Andreas Ehret
*/

#ifndef _SBR_MISC_H
#define _SBR_MISC_H

/* Sorting routines */
void Shellsort_int   (int *in, int n);

void AddLeft (int *vector, int *length_vector, int value);
void AddRight (int *vector, int *length_vector, int value);
void AddVecLeft (int *dst, int *length_dst, int *src, int length_src);
void AddVecRight (int *dst, int *length_vector_dst, int *src, int length_src);
#endif
