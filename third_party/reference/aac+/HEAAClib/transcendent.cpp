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

 $Id: transcendent.c,v 1.1.2.11 2004/05/30 13:59:40 ehr Exp $

*******************************************************************************/
/*!
  \file
  \brief Implementation of math functions  $Revision: 1.1.2.11 $
*/
   
#include <math.h>
#include <assert.h>
#include "FloatFR.h"

#include "flc.h" /* the 3GPP instrumenting tool */

static float logDualisTable[LOG_DUALIS_TABLE_SIZE];

/*
  Creates lookup tables for some arithmetic functions
*/
void FloatFR_Init(void)
{
  int i;

  FLC_sub_start("FloatFR_Init");

  MULT(1); STORE(1);
  logDualisTable[0] = -1.0f; /* actually, ld 0 is not defined */

  PTR_INIT(1); /* logDualisTable[] */
  LOOP(1);
  for (i=1; i<LOG_DUALIS_TABLE_SIZE; i++) {
    TRANS(1); MULT(1); /* xxx * (1 / 0.30103) */ STORE(1);
    logDualisTable[i] = (float) ( log((float)i)/log(2.0f) );
  }
  
  FLC_sub_end();
}


/*
  The table must have been created before by FloatFR_Init().
  The valid range for a is 1 to LOG_DUALIS_TABLE_SIZE.
  For a=0, the result will be -1 (should be -inf).

  returns ld(a)
*/
float FloatFR_logDualis(int a)  /* Index for logarithm table */
{
  assert( a>=0 && a<LOG_DUALIS_TABLE_SIZE );

  FLC_sub_start("FloatFR_logDualis");
  INDIRECT(1);
  FLC_sub_end();

  return logDualisTable[a];
}


/*
  The function FloatFR_Init() must have been called before use.
  The valid range for a and b is 1 to LOG_DUALIS_TABLE_SIZE.

  returns ld(a/b)
*/
float FloatFR_getNumOctaves(int a, /* lower band */
                            int b) /* upper band */
{
  FLC_sub_start("FloatFR_getNumOctaves");
  FUNC(1); FUNC(1); ADD(1);
  FLC_sub_end();

  return (FloatFR_logDualis(b) - FloatFR_logDualis(a));
}

