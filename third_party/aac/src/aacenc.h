/****************************************************************************

                      (C) copyright Coding Technologies (2004)
                               All Rights Reserved

 This software module was developed by Coding Technologies (CT). This is
 company confidential information and the property of CT, and can not be
 reproduced or disclosed in any form without written authorization of CT.

 Those intending to use this software module for other purposes are advised
 that this infringe existing or pending patents. CT has no liability for
 use of this software module or derivatives thereof in any implementation.
 Copyright is not released for any means. CT retains full right to use the
 code for its own purpose, assign or sell the code to a third party and to
 inhibit any user or third party from using the code. This copyright notice
 must be included in all copies or derivative works.

 $Id: aacenc.h,v 1.4.2.10 2004/05/30 14:15:18 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  fast aac coder interface library functions $Revision: 1.4.2.10 $
  \author M. Schug / A. Groeschel
*/

#ifndef _aacenc_h_
#define _aacenc_h_

/* here we distinguish between stereo and the mono only encoder */
#ifdef MONO_ONLY
#define MAX_CHANNELS        1
#else
#define MAX_CHANNELS        2
#endif

#define AACENC_BLOCKSIZE    1024   /*! encoder only takes BLOCKSIZE samples at a time */
#define AACENC_TRANS_FAC    8      /*! encoder short long ratio */
#define AACENC_PCM_LEVEL    1.0    /*! encoder pcm 0db refernence */


/*-------------------------- defines --------------------------------------*/

#define BUFFERSIZE 1024     /* anc data */

/*-------------------- structure definitions ------------------------------*/

typedef  struct {
  int   sampleRate;            /* audio file sample rate */
  int   bitRate;               /* encoder bit rate in bits/sec */
  int   nChannelsIn;           /* number of channels on input (1,2) */
  int   nChannelsOut;          /* number of channels on output (1,2) */
  int   bandWidth;             /* core coder audio bandwidth in Hz */
} AACENC_CONFIG;

struct AAC_ENCODER;

/*
 * public ancillary
 *
 */

/*-----------------------------------------------------------------------------

     functionname: AacInitDefaultConfig
     description:  gives reasonable default configuration
     returns:      ---

 ------------------------------------------------------------------------------*/
void AacInitDefaultConfig(AACENC_CONFIG *config);

/*---------------------------------------------------------------------------

    functionname:AacEncOpen
    description: allocate and initialize a new encoder instance
    returns:     AACENC_OK if success

  ---------------------------------------------------------------------------*/

int  AacEncOpen
(  struct AAC_ENCODER**     phAacEnc,       /* pointer to an encoder handle, initialized on return */
   const  AACENC_CONFIG     config          /* pre-initialized config struct */
);

int AacEncEncode(struct AAC_ENCODER  *hAacEnc,
                 float               *timeSignal,
                 unsigned int        timeInStride,
                 const unsigned char *ancBytes,      /*!< pointer to ancillary data bytes */
                 unsigned int        *numAncBytes,   /*!< number of ancillary Data Bytes, send as fill element  */
                 unsigned int        *outBytes,      /*!< pointer to output buffer            */
                 int                 *numOutBytes    /*!< number of bytes in output buffer */
                 );


/*---------------------------------------------------------------------------

    functionname:AacEncClose
    description: deallocate an encoder instance

  ---------------------------------------------------------------------------*/

void AacEncClose (struct AAC_ENCODER* hAacEnc); /* an encoder handle */


#endif /* _aacenc_h_ */
