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
 
 $Id: qc_data.h,v 1.5.2.10 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Quantizing & coding data $Revision: 1.5.2.10 $
  \author M. Werner
*/
#ifndef _QC_DATA_H
#define _QC_DATA_H

#include "psy_const.h"
#include "dyn_bits.h"
#include "adj_thr_data.h"


typedef enum {
  ID_SCE=0,     /* Single Channel Element   */
  ID_CPE=1,     /* Channel Pair Element     */
  ID_FIL=6,
  ID_END=7
} ELEMENT_TYPE;

typedef struct {
  ELEMENT_TYPE elType;
  int instanceTag;
  int nChannelsInEl;
  int ChannelIndex[MAX_CHANNELS];
} ELEMENT_INFO;

typedef struct {
  int paddingRest;
} PADDING;


/* Quantizing & coding stage */

struct QC_INIT{
  ELEMENT_INFO* elInfo;
  int maxBits;     /* maximum number of bits in reservoir  */
  int averageBits; /* average number of bits we should use */
  int bitRes;
  float meanPe;
  int chBitrate;
  int invQuant;
  float maxBitFac;
  int bitrate;

  PADDING padding;

  QC_INIT(){
	  elInfo = NULL;
	  maxBits = 0;
	  averageBits = 0;
	  bitRes = 0;
	  meanPe = 0;
	  chBitrate = 0;
	  invQuant = 0;
	  maxBitFac = 0;
	  bitrate = 0;
  }
};

typedef struct
{
  short          quantSpec[FRAME_LEN_LONG];       /* [FRAME_LEN_LONG];                            */
  unsigned short maxValueInSfb[MAX_GROUPED_SFB];   /* [MAX_GROUPED_SFB];                           */

  short          scf[MAX_GROUPED_SFB];             /* [MAX_GROUPED_SFB];                           */
  int          globalGain;
  int          groupingMask;
  SECTION_DATA sectionData;
  int          windowShape;    /* doesn't really belong here. Convenient, though. */
} QC_OUT_CHANNEL;

typedef struct
{
  int          staticBitsUsed; /* for verification purposes */
  int          dynBitsUsed;    /* for verification purposes */
  float        pe;
  int          ancBitsUsed;
  int          fillBits;
} QC_OUT_ELEMENT;

typedef struct
{
  QC_OUT_CHANNEL  qcChannel[MAX_CHANNELS];
  QC_OUT_ELEMENT  qcElement;
  int          totStaticBitsUsed; /* for verification purposes */
  int          totDynBitsUsed;    /* for verification purposes */
  int          totAncBitsUsed;    /* for verification purposes */
  int          totFillBits;
  int          alignBits;
  int          bitResTot;
  int          averageBitsTot;
} QC_OUT;

typedef struct {
  int chBitrate;
  int averageBits;               /* brutto -> look ancillary.h */
  int maxBits;
  int bitResLevel;
  int maxBitResBits;
  float relativeBits;            /* Bits relative to total Bits*/
} ELEMENT_BITS;

typedef struct
{
  /* this is basically struct QC_INIT */
  int averageBitsTot;
  int maxBitsTot;
  int globStatBits;
  int nChannels;
  int bitResTot;

  float maxBitFac;

  PADDING   padding;

  ELEMENT_BITS  elementBits;
  ADJ_THR_STATE adjThr;

} QC_STATE;

#endif /* _QC_DATA_H */




