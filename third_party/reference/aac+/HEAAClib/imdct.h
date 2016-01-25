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

 $Id: imdct.h,v 1.4.6.1 2004/02/16 15:52:53 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  inverse transformation  $Revision: 1.4.6.1 $
  \author zerok 
*/

#ifndef IMDCT_H
#define IMDCT_H

void CLongBlock_InverseTransform(float data[]);

void CShortBlock_InverseTransform(float data[]);

#endif /* #ifndef IMDCT_H */
