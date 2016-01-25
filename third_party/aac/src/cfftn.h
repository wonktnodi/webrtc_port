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

 $Id: cfftn.h,v 1.1.4.1 2004/02/17 09:58:01 hfm Exp $

*******************************************************************************/
/*!
  \file
  \brief  CFFTN header file $Revision: 1.1.4.1 $
  \author M. Werner
*/

#ifndef __cfftn_h
#define __cfftn_h


void forwardFFT( float *data, int length );

int cfftn(float Re[],
          float Im[],
          int  nTotal,
          int  nPass,
          int  nSpan,
          int  iSign);

int CFFTN(float *afftData,int len, int isign);
int CFFTNRI(float *afftDataReal,float *afftDataImag,int len, int isign);

int CFFTN_NI(float *InRealData,
              float *InImagData,
              float *OutRealData,
              float *OurImagData,
              int len, int isign);


#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif


#endif
