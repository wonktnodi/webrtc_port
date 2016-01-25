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
 
 $Id: tns_param.c,v 1.1.2.6 2004/02/16 21:05:59 ehr Exp $
 
****************************************************************************/
/*!
  \file 
  \brief  Temporal Noise Shaping parameters $Revision: 1.1.2.6 $
  \author A. Groeschel
*/
#include <stdio.h>
#include "aac_rom_enc.h"

#include "flc.h" /* the 3GPP instrumenting tool */


/*****************************************************************************

    functionname: GetTnsParam
    description:  Get threshold calculation parameter set that best matches
                  the bit rate
    returns:      the parameter set
    input:        blockType, bitRate
    output:       the parameter set

*****************************************************************************/
int GetTnsParam(TNS_CONFIG_TABULATED *tnsConfigTab, int bitRate, int channels, int blockType) {

  unsigned int i;

  FLC_sub_start("GetTnsParam");

  PTR_INIT(1); /* tnsInfoTab[] */
  
  BRANCH(1); LOGIC(1);
  if (tnsConfigTab == NULL)
    return 1;

  MOVE(1);
  tnsConfigTab->threshOn = -1.0f;

  LOOP(1);
  for(i = 0; i < sizeof(tnsInfoTab)/sizeof(TNS_INFO_TAB); i++) {

    ADD(2); LOGIC(1); BRANCH(1);
    if((bitRate >= tnsInfoTab[i].bitRateFrom) && bitRate <= tnsInfoTab[i].bitRateTo) {

      BRANCH(2);
      switch(blockType) {

      case LONG_WINDOW :
        BRANCH(2);
        switch(channels) {
        case 1 :
          MOVE(1);
          *tnsConfigTab=*tnsInfoTab[i].paramMono_Long;
          break;
        case 2 :
          MOVE(1);
          *tnsConfigTab=*tnsInfoTab[i].paramStereo_Long;
          break;
        }
      break;

      case SHORT_WINDOW :
        BRANCH(2);
        switch(channels) {
        case 1 :
          BRANCH(1);
          MOVE(1);
          *tnsConfigTab=*tnsInfoTab[i].paramMono_Short;
          break;
        case 2 :
          MOVE(1);
          *tnsConfigTab=*tnsInfoTab[i].paramStereo_Short;
          break;
        }

        break;
      }
    }
  }

  BRANCH(1); LOGIC(1);
  if (tnsConfigTab->threshOn == -1.0f) {
    return 1;
  }

  FLC_sub_end();
  return 0;
}

/*****************************************************************************

    functionname: GetTnsMaxBands
    description:  sets tnsMaxSfbLong, tnsMaxSfbShort according to sampling rate
    returns:
    input:        samplingRate, profile, granuleLen
    output:       tnsMaxSfbLong, tnsMaxSfbShort

*****************************************************************************/
void GetTnsMaxBands(int samplingRate, int blockType, int* tnsMaxSfb){
  unsigned int i;

  FLC_sub_start("GetTnsMaxBands");

  MULT(1); STORE(1);
  *tnsMaxSfb=-1;

  PTR_INIT(1); /* tnsMaxBandsTab[] */
  LOOP(1);
  for(i=0;i<sizeof(tnsMaxBandsTab)/sizeof(TNS_MAX_TAB_ENTRY);i++){

    ADD(1); BRANCH(1);
    if(samplingRate == tnsMaxBandsTab[i].samplingRate){

        ADD(1); BRANCH(1); MOVE(1);
        *tnsMaxSfb=(blockType==2)?tnsMaxBandsTab[i].maxBandShort:
                                  tnsMaxBandsTab[i].maxBandLong;
        break;
    }
  }

  FLC_sub_end();
}
