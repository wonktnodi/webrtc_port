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

 $Id: lpp_tran.h,v 1.9.6.2 2004/02/17 15:36:17 jr Exp $

*******************************************************************************/
/*
  \file
  \brief  Low Power Profile Transposer, $Revision: 1.9.6.2 $
  \author Markus Werner
*/

#ifndef _LPP_TRANS_H
#define _LPP_TRANS_H

#include "sbrdecsettings.h"
#include "sbrdecoder.h"

#define MAX_NUM_PATCHES 6
#define GUARDBANDS      0
#define SHIFT_START_SB  1

typedef enum
{
  INVF_OFF = 0,
  INVF_LOW_LEVEL,
  INVF_MID_LEVEL,
  INVF_HIGH_LEVEL,
  INVF_SWITCHED
}
INVF_MODE;


typedef struct {
  unsigned char  sourceStartBand;
  unsigned char  sourceStopBand;
  unsigned char  guardStartBand;

  unsigned char  targetStartBand;
  unsigned char  targetBandOffs;
  unsigned char  numBandsInPatch;
} PATCH_PARAM;


typedef struct {
  float  off;
  float  transitionLevel;
  float  lowLevel;
  float  midLevel;
  float  highLevel;
} WHITENING_FACTORS;



typedef struct {
  unsigned char  nCols;
  unsigned char  noOfPatches;
  unsigned char  lbStartPatching;
  unsigned char  lbStopPatching;
  unsigned char  bwBorders[MAX_NUM_NOISE_VALUES];
  PATCH_PARAM    patchParam[MAX_NUM_PATCHES];
  WHITENING_FACTORS whFactors;

} TRANSPOSER_SETTINGS;


typedef struct
{
  TRANSPOSER_SETTINGS *pSettings;
  float  bwVectorOld[MAX_NUM_PATCHES];
  float  *lpcFilterStatesReal[LPC_ORDER];
#ifndef LP_SBR_ONLY
  float  *lpcFilterStatesImag[LPC_ORDER];
#endif
  float sbr_LpcFilterStatesReal[MAXNRQMFCHANNELS][LPC_ORDER][NO_ANALYSIS_CHANNELS];
  float sbr_LpcFilterStatesImag[MAXNRQMFCHANNELS][LPC_ORDER][NO_ANALYSIS_CHANNELS];

  TRANSPOSER_SETTINGS sbr_TransposerSettings;
}
SBR_LPP_TRANS;

typedef SBR_LPP_TRANS *HANDLE_SBR_LPP_TRANS;


void lppTransposer (HANDLE_SBR_LPP_TRANS hLppTrans,
                    float **qmfBufferReal,
#ifndef LP_SBR_ONLY
                    float **qmfBufferImag,
#endif
                    float *degreeAlias,
                    int timeStep,
                    int firstSlotOffset,
                    int lastSlotOffset,
                    unsigned char nInvfBands,
                    INVF_MODE *sbr_invf_mode,
                    INVF_MODE *sbr_invf_mode_prev,
                    int bUseLP
                    );


int
createLppTransposer (HANDLE_SBR_LPP_TRANS hLppTrans,
                     unsigned char highBandStartSb,
                     unsigned char *v_k_master,
                     unsigned char numMaster,
                     unsigned char usb,
                     unsigned char nCols,
                     unsigned char* noiseBandTable,
                     unsigned char noNoiseBands,
                     unsigned short fs,
                     unsigned char chan);


int
resetLppTransposer (HANDLE_SBR_LPP_TRANS hLppTrans,
                    unsigned char xposctrl,
                    unsigned char highBandStartSb,
                    unsigned char *v_k_master,
                    unsigned char numMaster,
                    unsigned char* noiseBandTable,
                    unsigned char noNoiseBands,
                    unsigned char usb,
                    unsigned short fs);



#endif /* _LPP_TRANS_H */

