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
 
 $Id: block_switch.h,v 1.4.2.3 2004/02/16 20:02:18 ehr Exp $
 
****************************************************************************/
/*!
  \file
  \brief  Block Switching $Revision: 1.4.2.3 $
  \author M. Werner
*/
#ifndef _BLOCK_SWITCH_H
#define _BLOCK_SWITCH_H

#define BLOCK_SWITCHING_IIR_LEN 2    
#define BLOCK_SWITCH_WINDOWS    TRANS_FAC               /* number of windows for energy calculation */
#define BLOCK_SWITCH_WINDOW_LEN FRAME_LEN_SHORT         /* minimal granularity of energy calculation */


typedef struct{
  float invAttackRatio;
  int   windowSequence;
  int   nextwindowSequence;
  int   attack;
  int   lastattack;
  int   attackIndex;
  int   lastAttackIndex;
  int   noOfGroups;
  int   groupLen[TRANS_FAC];
  float windowNrg[2][BLOCK_SWITCH_WINDOWS];     /* time signal energy in Subwindows (last and current) */
  float windowNrgF[2][BLOCK_SWITCH_WINDOWS];    /* filtered time signal energy in segments (last and current) */
  float iirStates[BLOCK_SWITCHING_IIR_LEN];     /* filter delay-line */
  float maxWindowNrg;                           /* max energy in subwindows */
  float accWindowNrg;                           /* recursively accumulated windowNrgF */
}BLOCK_SWITCHING_CONTROL;

int   InitBlockSwitching(BLOCK_SWITCHING_CONTROL *blockSwitchingControl,
                         const int bitRate, const int nChannels);

int   BlockSwitching(BLOCK_SWITCHING_CONTROL *blockSwitchingControl,
                     float *timeSignal,
                     int chIncrement);

int   SyncBlockSwitching(BLOCK_SWITCHING_CONTROL *blockSwitchingControlLeft,
                         BLOCK_SWITCHING_CONTROL *blockSwitchingControlRight,
                         const int noOfChannels);

#endif  /* #ifndef _BLOCK_SWITCH_H */
