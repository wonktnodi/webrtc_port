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

 $Id: block.h,v 1.6.6.3 2004/02/16 15:52:53 ehr Exp $

****************************************************************************/
/*!
  \file
  \brief  long/short decoding  $Revision: 1.6.6.3 $
  \author zerokl
*/

#ifndef BLOCK_H
#define BLOCK_H

#include "channelinfo.h"
#include "FFR_bitbuffer.h"

enum
{
  MaximumBinsLong = FRAME_SIZE,
  MaximumBinsShort = FRAME_SIZE/8,
  MaximumScaleFactorBandsLong = 64,
  MaximumScaleFactorBandsShort = 16,
  MaximumGroups = MaximumWindows,

  ZERO_HCB = 0,
  ESCBOOK  = 11,
  NSPECBOOKS = ESCBOOK + 1,
  BOOKSCL    = NSPECBOOKS,
  NOISE_HCB      = 13,
  INTENSITY_HCB2 = 14,
  INTENSITY_HCB  = 15,

  LD_MANT=         12,
  MANT_SIZE=       4096,
  LD_POW_TAB=      8,
  POW_TAB_SIZE=    256,
  LD_DELTA=        4,
  DELTA_SIZE=      16,
  TNS_SCALE=        2,

  HuffmanBits = 2,
  HuffmanEntries = (1 << HuffmanBits)
};

enum {
  Size01 = (MaximumBinsLong/16),
  Size02 = ( 2*Size01),
  Size03 = ( 3*Size01),
  Size04 = ( 4*Size01),
  Size05 = ( 5*Size01),
  Size06 = ( 6*Size01),
  Size07 = ( 7*Size01),
  Size08 = ( 8*Size01),
  Size09 = ( 9*Size01),
  Size10 = (10*Size01),
  Size11 = (11*Size01),
  Size12 = (12*Size01),
  Size13 = (13*Size01),
  Size14 = (14*Size01),
  Size15 = (15*Size01),
  Size16 = (16*Size01)
};

typedef struct tagCodeBookDescription
{
  char Dimension;
  char numBits;
  char Offset;
  const unsigned short (*CodeBook)[HuffmanEntries];

} CodeBookDescription;

float CBlock_EvalPow43(int a);
float CBlock_Quantize(int value,int scfMod,int scale);

int CBlock_GetEscape(HANDLE_BIT_BUF bs,const int q);
int CBlock_UnpackIndex(int idx, int *qp, const CodeBookDescription *hcb);
int CBlock_DecodeHuffmanWord(HANDLE_BIT_BUF bs,const unsigned short (*CodeBook) [HuffmanEntries]);

void CShortBlock_Init(CAacDecoderChannelInfo *pAacDecoderChannelInfo);

int CLongBlock_Read(HANDLE_BIT_BUF bs,CAacDecoderChannelInfo *pAacDecoderChannelInfo,unsigned char global_gain);
int CShortBlock_Read(HANDLE_BIT_BUF bs,CAacDecoderChannelInfo *pAacDecoderChannelInfo,unsigned char global_gain);

void CLongBlock_ReadScaleFactorData(HANDLE_BIT_BUF bs,CAacDecoderChannelInfo *pAacDecoderChannelInfo,unsigned char global_gain);
int CLongBlock_ReadSectionData(HANDLE_BIT_BUF bs,CAacDecoderChannelInfo *pAacDecoderChannelInfo);
int CLongBlock_ReadSpectralData(HANDLE_BIT_BUF bs,CAacDecoderChannelInfo *pAacDecoderChannelInfo);

void CShortBlock_ReadScaleFactorData(HANDLE_BIT_BUF bs,CAacDecoderChannelInfo *pAacDecoderChannelInfo,unsigned char global_gain);
int CShortBlock_ReadSectionData(HANDLE_BIT_BUF bs,CAacDecoderChannelInfo *pAacDecoderChannelInfo);
int CShortBlock_ReadSpectralData(HANDLE_BIT_BUF bs,CAacDecoderChannelInfo *pAacDecoderChannelInfo);

void ApplyTools(AACDECODER self,CAacDecoderChannelInfo *pAacDecoderChannelInfo[], int channel);

void CLongBlock_FrequencyToTime(CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,CAacDecoderChannelInfo* pAacDecoderChannelInfo,float [],const int);
void CShortBlock_FrequencyToTime(CAacDecoderStaticChannelInfo *pAacDecoderStaticChannelInfo,CAacDecoderChannelInfo *pAacDecoderChannelInfo,float [],const int);

void Lap1(float *coef,float *prev,float *out,const float *window,int size,int stride);
void Lap2(float *coef,float *prev,float *out,const float *window,int size,int stride);

#endif /* #ifndef BLOCK_H */
