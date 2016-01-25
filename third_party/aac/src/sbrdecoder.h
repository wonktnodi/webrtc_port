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

 $Id: sbrdecoder.h,v 1.4.6.7 2004/05/30 14:11:14 ehr Exp $

*******************************************************************************/
/*!
  \file
  \brief  SBR decoder frontend prototypes and definitions $Revision: 1.4.6.7 $
  \author Roland Bitto
*/

#ifndef __SBRDECODER_H
#define __SBRDECODER_H

#define SBR_EXTENSION          13  /* 1101 */
#define SBR_EXTENSION_CRC      14  /* 1110 */

#define MAXNRELEMENTS 2
#define MAXNRSBRCHANNELS MAXNRELEMENTS

#ifdef MONO_ONLY
#define MAXNRQMFCHANNELS 1
#else
#define MAXNRQMFCHANNELS MAXNRSBRCHANNELS
#endif

#define MAXSBRBYTES 128        /* have to be correctly determinated */


typedef enum
{
  SBRDEC_OK = 0,
  SBRDEC_CONCEAL,
  SBRDEC_NOSYNCH,
  SBRDEC_ILLEGAL_PROGRAM,
  SBRDEC_ILLEGAL_TAG,
  SBRDEC_ILLEGAL_CHN_CONFIG,
  SBRDEC_ILLEGAL_SECTION,
  SBRDEC_ILLEGAL_SCFACTORS,
  SBRDEC_ILLEGAL_PULSE_DATA,
  SBRDEC_MAIN_PROFILE_NOT_IMPLEMENTED,
  SBRDEC_GC_NOT_IMPLEMENTED,
  SBRDEC_ILLEGAL_PLUS_ELE_ID,
  SBRDEC_CREATE_ERROR,
  SBRDEC_NOT_INITIALIZED
}
SBR_ERROR;

typedef enum
{
  SBR_ID_SCE = 0,
  SBR_ID_CPE,
  SBR_ID_CCE,
  SBR_ID_LFE,
  SBR_ID_DSE,
  SBR_ID_PCE,
  SBR_ID_FIL,
  SBR_ID_END
}
SBR_ELEMENT_ID;

typedef struct
{
  int ElementID;
  int ExtensionType;
  int Payload;
  unsigned char Data[MAXSBRBYTES];
}
SBR_ELEMENT_STREAM;

typedef struct
{
  int NrElements;
  int NrElementsCore;
  SBR_ELEMENT_STREAM sbrElement[MAXNRELEMENTS]; /* for the delayed frame */
}
SBRBITSTREAM;


typedef struct SBR_DECODER_INSTANCE *SBRDECODER;


SBRDECODER openSBR (SBR_DECODER_INSTANCE &sbrDecoderInstance, int sampleRate, int samplesPerFrame, int bDownSample, int bApplyQmfLp ) ;

SBR_ERROR applySBR (SBRDECODER self,
                    SBRBITSTREAM * Bitstr,
                    float *TimeData,
					float *workBufferCore,
                    int *numChannels,
                    int frameOK,
                    int bDownSample,
                    int bBitstreamDownMix);
#endif
