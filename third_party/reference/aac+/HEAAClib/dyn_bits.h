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
 
 $Id: dyn_bits.h,v 1.2.2.1 2004/02/16 21:05:58 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  Noiseless coder module  $Revision: 1.2.2.1 $
  \author M.Werner
*/
#ifndef __DYN_BITS_H
#define __DYN_BITS_H

#include "psy_const.h"
#include "tns_enc.h"
#include "bit_cnt.h"
#include <string>

#define MAX_SECTIONS          MAX_GROUPED_SFB
#define SECT_ESC_VAL_LONG    31
#define SECT_ESC_VAL_SHORT    7
#define CODE_BOOK_BITS        4
#define SECT_BITS_LONG        5
#define SECT_BITS_SHORT       3

typedef struct
{
  int codeBook;
  int sfbStart;
  int sfbCnt;
  int sectionBits;              /* huff + si ! */
}
SECTION_INFO;

typedef struct __tagSECTION_DATA
{
  int blockType;
  int noOfGroups;
  int sfbCnt;
  int maxSfbPerGroup;
  int sfbPerGroup;
  int noOfSections;
  SECTION_INFO section[MAX_SECTIONS];
  int sideInfoBits;             /* sectioning bits       */
  int huffmanBits;              /* huffman    coded bits */
  int scalefacBits;             /* scalefac   coded bits */
  int firstScf;                 /* first scf to be coded */

  int sideInfoTabLong[MAX_SFB_LONG + 1];
  int sideInfoTabShort[MAX_SFB_SHORT + 1];

  __tagSECTION_DATA()
  {
	  extern int  calcSideInfoBits(int sfbCnt, int blockType);
	  for (int i = 0; i <= MAX_SFB_LONG; i++)
	  {
		  sideInfoTabLong[i] = calcSideInfoBits(i, LONG_WINDOW);
	  }

		/* sideInfoTabShort[] */
	  for (int i = 0; i <= MAX_SFB_SHORT; i++)
	  {
		  sideInfoTabShort[i] = calcSideInfoBits(i, SHORT_WINDOW);
	  }
  }
}
SECTION_DATA;


int  BCInit(void);

int  dynBitCount(const short *quantSpectrum,
                 const unsigned short *maxValueInSfb,
                 const signed  short  *scalefac,
                 const int blockType,
                 const int sfbCnt,
                 const int maxSfbPerGroup,
                 const int sfbPerGroup,
                 const int *sfbOffset,
                 SECTION_DATA * sectionData);

int  RecalcSectionInfo(SECTION_DATA * sectionData,
                       const unsigned short *maxValueInSfb,
                       const signed   short *scalefac,
                       const int blockType);

#endif
