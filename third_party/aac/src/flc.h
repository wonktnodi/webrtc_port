/*===================================================================*
 *                                                                   *
 * In order to assist the 3GPP in the PSS and MSS audio codec        *
 * selection ("The Purpose"), VoiceAge Corp. is providing 3GPP with  *
 * a copy of the complexity evaluation tools ("The Software"). The   *
 * Software is offered to 3GPP as a basis for discussion and should  *
 * not be considered as a binding proposal on VoiceAge Corp. or on   *
 * any other company.                                                *
 *                                                                   *
 * Any unauthorized copying or reproduction of this document for     *
 * monetary gain or any other Purpose without VoiceAge Corp's        *
 * express prior written consent is strictly prohibited.             *
 *                                                                   *
 * Nothing contained herein shall be construed as conferring any     *
 * license or right under any patent, whether or not the use of      *
 * information herein necessarily employs an invention of any        *
 * existing or later issued patent, or copyright. In addition,       *
 * VoiceAge Corp. makes no representation nor warranty in regard to  *
 * the accuracy, completeness or sufficiency of The Software, nor    *
 * shall VoiceAge Corp. be held liable for any damages whatsoever    *
 * relating to use of said Software.                                 *
 *                                                                   *
 *===================================================================*/

/*
 * flc.h
 *
 * Tools for estimating computational complexity and Program ROM.
 *
 * Copyright 2003 VoiceAge Corp. All Rights Reserved.
 *      Written by Joachim Thiemann,  April 11, 2003
 *
 * $Id: flc.h,v 1.2.2.6 2004/04/01 12:41:26 ehr Exp $
 */

#ifndef FLC_H
#define FLC_H

#include <stdio.h>

/* enable this define to apply instrumenation */
#define INSTRUMENTATION
#undef INSTRUMENTATION

/* enable this define to enable CT specific enhancements to the original VoiceAge Tool */
#define CT_INSTRUMENT
#undef CT_INSTRUMENT

/* NOTE: CHANGE THIS VALUE ACCORDING TO FRAME LENGTH OF CODE */
/* Frame lenght is defined in milliseconds */
/*#define FLC_FRAMELEN 46.4399F*/ /* corresponds to 44.1 kHz */
#define FLC_FRAMELEN 42.6666F /* corresponds to 48 kHz */


/* opcodes - this list must match the names in flc.c */
enum flc_fields
{
   FLC_NOP = 0,
   FLC_ADD,
   FLC_MULT,
   FLC_MAC,
   FLC_MOVE,
   FLC_STORE,
   FLC_LOGIC,
   FLC_SHIFT,
   FLC_BRANCH,
   FLC_DIV,
   FLC_SQRT,
   FLC_TRANS,                   /* transcendal fn (sin. cos) */
   FLC_FUNC,
   FLC_LOOP,
   FLC_INDIRECT,                /* indirect ref */
   FLC_PTR_INIT,
   FLC_MISC,                    /* For all other ops (eg. ABS) - weight 1 mem, 1 op */
   FLC_OPEND
};

#ifdef INSTRUMENTATION
#define _FLC(o,c)  {static int f=0; FLC_ops((o),(c)); if (!f) {f=1;FLC_mem((o),(c));}}
#else
#define _FLC(o,c)
#endif
#define ADD(c)          _FLC( FLC_ADD,      (c) )
#define MULT(c)         _FLC( FLC_MULT,     (c) )
#define MAC(c)          _FLC( FLC_MAC,      (c) )
#define MOVE(c)         _FLC( FLC_MOVE,     (c) )
#define STORE(c)        _FLC( FLC_STORE,    (c) )
#define LOGIC(c)        _FLC( FLC_LOGIC,    (c) )
#define SHIFT(c)        _FLC( FLC_SHIFT,    (c) )
#define BRANCH(c)       _FLC( FLC_BRANCH,   (c) )
#define DIV(c)          _FLC( FLC_DIV,      (c) )
#define SQRT(c)         _FLC( FLC_SQRT,     (c) )
#define TRANS(c)        _FLC( FLC_TRANS,    (c) )
#define FUNC(c)         _FLC( FLC_FUNC,     (c) )
#define LOOP(c)         _FLC( FLC_LOOP,     (c) )
#define INDIRECT(c)     _FLC( FLC_INDIRECT, (c) )
#define PTR_INIT(c)     _FLC( FLC_PTR_INIT, (c) )
#define MISC(c)         _FLC( FLC_MISC,     (c) )

/* Double Ops count as double the operations but same memory */
#define _FLCD(o,c)  {static int f=0; FLC_ops((o),2*(c)); if (!f) {f=1;FLC_mem((o),(c));}}
#define DADD(c)         _FLCD( FLC_ADD,     (c) )
#define DMULT(c)        _FLCD( FLC_MULT,    (c) )
#define DMOVE(c)        _FLCD( FLC_MOVE,    (c) )
#define DDIV(c)         _FLCD( FLC_DIV,     (c) )

/* external function prototypes */
#ifdef INSTRUMENTATION
void FLC_init(void);
void FLC_end(void);
void FLC_sub_start(char *name);
void FLC_sub_end(void);
void FLC_frame_update(void);
/* the most important fn */
void FLC_ops(int op, int count);
void FLC_mem(int op, int count);
#else
#define FLC_init()
#define FLC_end()
#define FLC_sub_start(name) 
#define FLC_sub_end()
#define FLC_frame_update()
#define FLC_ops(op, count)
#define FLC_mem(op, count)
#endif


#endif
