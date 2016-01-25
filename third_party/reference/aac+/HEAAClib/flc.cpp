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
 * flc.c
 *
 * Tools for estimating computational complexity and Program ROM.
 *
 * Copyright 2003 VoiceAge Corp. All Rights Reserved.
 *      Written by Joachim Thiemann,  April 11, 2003
 *
 * $Id: flc.c,v 1.1.2.7 2004/02/12 15:03:22 ehr Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "flc.h"

#ifdef INSTRUMENTATION

/* printing behavioral defines */
#define FLC_MAXPREFIXLEN  8
#define FLC_MAXTAGLEN     30
#define FLC_OPSCNT_LEN    17
#define FLC_OPCODE_LEN    6
#define FLC_PTR_CMP

/* data structures for storing the data */
struct FLC_Ops_Data
{
   /* list and sublist references */
   struct FLC_Ops_Data *next;
   struct FLC_Ops_Data *subfirst;
   struct FLC_Ops_Data *sublast;
   struct FLC_Ops_Data *parent;
   /* counting data fields */
   char name[FLC_MAXTAGLEN + 1];
   unsigned long invocations;
   unsigned long fn_total[5];
   int idxFnTotal;
#ifdef CT_INSTRUMENT
   unsigned long fn_accum_total;
#endif
   unsigned long fn_max;
   unsigned long op_cnt;
   unsigned long optable[FLC_OPEND];
};

struct FLC_Mem_Data
{
   struct FLC_Mem_Data *next;
   int ref;
   int type;
   int count;
   char context[FLC_MAXTAGLEN + 1];
};

typedef struct FLC_Ops_Data FLC_Ops_Data;
typedef struct FLC_Mem_Data FLC_Mem_Data;


static char *Op_Names[FLC_OPEND] = {
   "NOP",
   "ADD", "MULT", "MAC",
   "MOVE", "STORE", "LOGIC",
   "SHIFT", "BRNCH", "DIV",
   "SQRT", "TRANC", "FUNC",
   "LOOP", "IND", "PTR",
   "MISC"
};
static int Mem_Weights[FLC_OPEND] = {
   0,
   1, 1, 1,                     //   "ADD", "MULT", "MAC",
   1, 0, 1,                     //   "MOVE", "STORE", "LOGIC",
   1, 2, 2,                     //   "SHIFT", "BRNCH", "DIV",
   2, 2, 2,                     //   "SQRT", "TRANC", "FUNC",
   1, 2, 1,                     //   "LOOP", "IND", "PTR"
   1                            //   "MISC"
};
static int Ops_Weights[FLC_OPEND] = {
   0,
   1, 1, 1,                     //   "ADD", "MULT", "MAC",
   1, 1, 1,                     //   "MOVE", "STORE", "LOGIC",
   1, 2, 18,                    //   "SHIFT", "BRNCH", "DIV",
   25, 25, 2,                   //   "SQRT", "TRANC", "FUNC",
   1, 2, 1,                     //   "LOOP", "IND", "PTR"
   1                            //   "MISC"
};

/* This structure is only used for printing the memory sums */
struct FLC_Mem_Summaries
{
   struct FLC_Mem_Summaries *next;
   char name[FLC_MAXTAGLEN + 1];
   int counts[FLC_OPEND];
};
typedef struct FLC_Mem_Summaries FLC_Mem_Summaries;
static FLC_Mem_Summaries *FLC_Mem_find_function(FLC_Mem_Summaries *top,
                                                char *name);

/* global variables */
FLC_Ops_Data *ops_top;
FLC_Ops_Data *ops_current;
#ifdef CT_INSTRUMENT
FLC_Ops_Data *indiv_ops_top;
FLC_Ops_Data *indiv_ops_bottom;
FLC_Ops_Data *indiv_ops_current;
#endif
FLC_Ops_Data *ops_total;
FLC_Mem_Data *mem_top;
FILE *FLC_output;
unsigned long totals[FLC_OPEND];

static FLC_Ops_Data *search_Ops_list(FLC_Ops_Data *top, char *name);
#ifdef CT_INSTRUMENT
static void sort_Ops_list(FLC_Ops_Data *top);
#endif

static void FLC_Ops_header(void);
static void FLC_Ops_print(char *prefix, FLC_Ops_Data *pL);
static void FLC_Ops_line(char *p, FLC_Ops_Data *pL);

#ifdef CT_INSTRUMENT
static void FLC_AccumOps_header();
static void FLC_AccumOps_print(char *prefix, FLC_Ops_Data *pL);
static void FLC_AccumOps_line(char *p, FLC_Ops_Data *pL);

static void FLC_IndivOps_header();
static void FLC_IndivOps_print(char *prefix, FLC_Ops_Data *pL);

static void FLC_IndivOpsWmops_header();
static void FLC_IndivOpsWmops_print(char *prefix, FLC_Ops_Data *pL);
#endif

static void FLC_Mem_print(void);

static void FLC_print_sum(void);
static void FLC_print_frame_sum(void);


/*
 * FLC initialisation. Must be called at start, sets up all the
 * structures and resets counters.
 *
 * Called externally.
 */
#ifndef DONT_COUNT
void FLC_init()
{
   FLC_output = stderr;

   ops_top = (FLC_Ops_Data *) malloc(sizeof(struct FLC_Ops_Data));
   if (!ops_top)
      perror("Allocation mem (ops_top) for flc");
   ops_total = (FLC_Ops_Data *) malloc(sizeof(struct FLC_Ops_Data));
   if (!ops_total)
      perror("Allocation mem (ops_total) for flc");
#ifdef CT_INSTRUMENT
   indiv_ops_top = (FLC_Ops_Data *) malloc(sizeof(struct FLC_Ops_Data));
   if (!indiv_ops_top)
      perror("Allocation mem (indiv_ops_top) for flc");
#endif

   ops_current = ops_top;
#ifdef CT_INSTRUMENT
   indiv_ops_current = indiv_ops_top;
   indiv_ops_bottom  = indiv_ops_top;
#endif


   /* clear the structures */
   memset(ops_current, 0, sizeof(struct FLC_Ops_Data));
   strncpy(ops_current->name, "ROOT", FLC_MAXTAGLEN); ops_current->name[FLC_MAXTAGLEN] = '\0';
   ops_current->invocations++;
#ifdef CT_INSTRUMENT
   memset(indiv_ops_current, 0, sizeof(struct FLC_Ops_Data));
   strncpy(indiv_ops_current->name, "ROOT", FLC_MAXTAGLEN); indiv_ops_current->name[FLC_MAXTAGLEN] = '\0';
   indiv_ops_current->invocations++;
#endif
   memset(ops_total, 0, sizeof(struct FLC_Ops_Data));
   strncpy(ops_total->name, "TOTAL", FLC_MAXTAGLEN); ops_total->name[FLC_MAXTAGLEN] = '\0';
   ops_total->invocations = 0;  /* this field is actually the frame count */

   /* the mem count structure is initialised a NULL pointer */
   mem_top = NULL;
}

/*
 * FLC finishing routine.  Adds up totals, prints results.
 *
 * Called externally.
 */

void FLC_end()
{
   fprintf(FLC_output, "\nCall Graph and total ops per function\n\n");
   FLC_Ops_header();
   FLC_Ops_print("", ops_top);

#ifdef CT_INSTRUMENT
   fprintf(FLC_output, "\nCall Graph and total accumulated ops per function\n\n");
   FLC_AccumOps_header();
   FLC_AccumOps_print("", ops_top);

   sort_Ops_list(indiv_ops_top);

   fprintf(FLC_output, "\nCall Graph and total ops per individual function (in cycles)\n\n");
   FLC_IndivOps_header();
   FLC_IndivOps_print("", indiv_ops_top);

   fprintf(FLC_output, "\nCall Graph and total ops per individual function (in Wmops)\n\n");
   FLC_IndivOpsWmops_header();
   FLC_IndivOpsWmops_print("", indiv_ops_top);
#endif

   fprintf(FLC_output, "\nProgram Memory Usage by Function\n\n");
   FLC_Mem_print();

   FLC_print_sum();
   if (ops_total->invocations != 0)
   {
      FLC_print_frame_sum();
   }
}

/*
 * FLC Subroutine or subsection start.  This changes the context so
 * that separate counts are kept for each subroutine and subsection.
 *
 * Called externally, must be matched with a FLC_sub_end()
 */

void FLC_sub_start(char *name)
{
   FLC_Ops_Data *newleaf;
   FLC_Ops_Data *temp;

   temp = search_Ops_list(ops_current->subfirst, name);

#ifdef DEBUG
   fprintf(stderr, "Context Switch: \"%s\"\n", name);
#endif

   if (!temp)
   {

      /* No context of that type in this function found. */

      newleaf = (FLC_Ops_Data *) malloc(sizeof(struct FLC_Ops_Data));
      memset(newleaf, 0, sizeof(struct FLC_Ops_Data));
      if (!newleaf)
         perror("Allocation mem for flc");

      newleaf->next = NULL;
      newleaf->subfirst = NULL;
      newleaf->sublast = NULL;
      newleaf->parent = ops_current;

      if (ops_current->subfirst == NULL)
      {
         ops_current->subfirst = newleaf;
         ops_current->sublast = newleaf;
      }
      else
      {
         ops_current->sublast->next = newleaf;
         ops_current->sublast = newleaf;
      }

      /* now switch contexts to new function */
      ops_current = newleaf;

      /* set the name pointer */
      strncpy(ops_current->name, name, FLC_MAXTAGLEN); ops_current->name[FLC_MAXTAGLEN] = '\0';
      ops_current->invocations++;

      /* clear some fields */
      ops_current->fn_max = 0;
      // ops_current->op_cnt = 0;
      // ops_current->fn_total = 0;
      // memset(&(ops_current->optable), 0, sizeof(unsigned int)*FLC_OPEND);

   }
   else
   {

      /* this context has been previously initialised */

      ops_current = temp;
      ops_current->invocations++;
   }

#ifdef CT_INSTRUMENT
   /* now do the pratically the same for the individual functions list */

   temp = search_Ops_list(indiv_ops_top, name);

   if (!temp)
   {

      /* No context of that type in this function found. */

      newleaf = (FLC_Ops_Data *) malloc(sizeof(struct FLC_Ops_Data));
      memset(newleaf, 0, sizeof(struct FLC_Ops_Data));
      if (!newleaf)
         perror("Allocation mem for flc");

      newleaf->next = NULL;
      newleaf->subfirst = NULL;
      newleaf->sublast = NULL;
      newleaf->parent = indiv_ops_bottom;

      indiv_ops_bottom->next = newleaf;
      indiv_ops_bottom = newleaf;

      /* now switch contexts to new function */
      indiv_ops_current = newleaf;

      /* set the name pointer */
      strncpy(indiv_ops_current->name, name, FLC_MAXTAGLEN); indiv_ops_current->name[FLC_MAXTAGLEN] = '\0';
      indiv_ops_current->invocations++;

      /* clear some fields */
      indiv_ops_current->fn_max = 0;
      // ops_current->op_cnt = 0;
      // ops_current->fn_total = 0;
      // memset(&(ops_current->optable), 0, sizeof(unsigned int)*FLC_OPEND);
   }
   else
   {

      /* this context has been previously initialised */

      indiv_ops_current = temp;
      indiv_ops_current->invocations++;
   }
#endif

}

/*
 * Match to the above.  Finishes up a subroutine context,
 * and computes totals
 */

void FLC_sub_end()
{
#ifdef CT_INSTRUMENT
  /* update counts for parents in the tree */
  FLC_Ops_Data *ops_parent;
#endif

   if (ops_current->parent == NULL)
   {
      fprintf(stderr, "ERROR: fell off stack in FLC_sub_end!\n");
   }
   else
   {
      /* Handle max counts per subroutine */
      if (ops_current->op_cnt > ops_current->fn_max)
         ops_current->fn_max = ops_current->op_cnt;
#ifdef CT_INSTRUMENT
      if (ops_current->op_cnt > indiv_ops_current->fn_max)
         indiv_ops_current->fn_max = ops_current->op_cnt;
#endif


      /* add current counts to per-fn total */
      ops_current->fn_total[0] += ops_current->op_cnt;
#ifdef CT_INSTRUMENT
      ops_current->fn_accum_total += ops_current->op_cnt;
      indiv_ops_current->fn_total[0] += ops_current->op_cnt;
      indiv_ops_current->fn_accum_total += ops_current->op_cnt;
#endif


#ifdef CT_INSTRUMENT
      /* update counts for parents in the tree */
      ops_parent = ops_current->parent;
      while (ops_parent) {
        FLC_Ops_Data *indiv_ops_parent;

        ops_parent->fn_accum_total += ops_current->op_cnt;

        indiv_ops_parent = search_Ops_list(indiv_ops_top, ops_parent->name);
        indiv_ops_parent->fn_accum_total += ops_current->op_cnt;

        ops_parent = ops_parent->parent;
      }
#endif

      /* add the current counts to the total program count */
      ops_total->op_cnt += ops_current->op_cnt;

      /* reset "current" counts */
      ops_current->op_cnt = 0;
#ifdef CT_INSTRUMENT
      indiv_ops_current->op_cnt = 0;
#endif

      /* finally, switch context back to higher frame */
      ops_current = ops_current->parent;
#ifdef CT_INSTRUMENT
      indiv_ops_current = search_Ops_list(indiv_ops_top, ops_current->name);
#endif
   }
}

void FLC_frame_update()
{
   ops_total->invocations++;

   if (ops_total->op_cnt > ops_total->fn_max)
     ops_total->fn_max = ops_total->op_cnt;

   if ( ops_total->fn_total[ops_total->idxFnTotal] > 4000000000u )
     ops_total->idxFnTotal++;

   ops_total->fn_total[ops_total->idxFnTotal] += ops_total->op_cnt;
   
   ops_total->op_cnt = 0;
}

/* the ops count routine */
void FLC_ops(int op, int c)
{
   if (op == FLC_FUNC)
   {
      /* the "FUNC" opcode is special in the handling of the argument */
      ops_current->op_cnt += Ops_Weights[op] + c;
      ops_current->optable[op]++;
   }
   else
   {
      ops_current->op_cnt += Ops_Weights[op] * c;
      ops_current->optable[op] += c;
   }
}

/* the memory count routine */
void FLC_mem(int op, int c)
{
   FLC_Mem_Data *temp;

   /* c is the no. or args */
   if (op == FLC_FUNC)
      c = 1;

   temp = (FLC_Mem_Data*) malloc(sizeof(struct FLC_Mem_Data));
   if (!temp)
   {
      perror("Allocating memory for mem counter");
      exit(1);
   }
   temp->next = mem_top;
   temp->count = c;
   temp->type = op;
   strncpy(temp->context, ops_current->name, FLC_MAXTAGLEN); temp->context[FLC_MAXTAGLEN] = '\0';

   mem_top = temp;

   return;
}
#else
void FLC_init() {};
void FLC_end() {};
void FLC_sub_start(char *name) {};
void FLC_sub_end() {};
void FLC_frame_update() {};

/* the most important fn */
void FLC_ops(int op, int count) {};
void FLC_mem(int op, int count) {};
#endif


/**************************************************************************
 * From here, there are only internal (static) functions                  *
 **************************************************************************/

/*
 * search a FLC_Ops_Data list for an item according to name pointer
 */

static FLC_Ops_Data *search_Ops_list(FLC_Ops_Data *top, char *name)
{
   FLC_Ops_Data *pL;

   /* walk through the list until we find the match or fall off the
      end */
   for (pL = top; pL; pL = pL->next)
   {
      /* Note: use one of the two below */
      /* if (pL->name == name)
         return pL; */
      if (!strncmp(pL->name, name, FLC_MAXTAGLEN )) return pL;
   }

   return pL;
}

#ifdef CT_INSTRUMENT
/*
 * sort the FLC_Ops_Data list according to accumulated cycles
 * (just a very bad sorting algorithm...)
 */

static void sort_Ops_list(FLC_Ops_Data *top)
{
   FLC_Ops_Data *pL, *pL2;

   pL = top->next;
   top->next =  NULL;

   for ( ; pL; )
   {
     FLC_Ops_Data *pL_next    = pL->next;
     FLC_Ops_Data *pL2_next;

     for (pL2 = top; pL2; ) {

       pL2_next   = pL2->next;

       if (pL2_next) {
         if (pL->fn_accum_total <= pL2->fn_accum_total && 
             pL->fn_accum_total > pL2_next->fn_accum_total) {

           break;
         }
       } else {
         indiv_ops_bottom = pL;
         break;
       }

       pL2 = pL2_next;
     }

     pL2->next          = pL;
     if (pL2_next)
       pL2_next->parent = pL;

     pL->parent = pL2;
     pL->next   = pL2_next;

     pL = pL_next;
   }
}
#endif

/**************************************************************
 * below this are functions for printing the summaries        *
 **************************************************************/

#define FLC_PFIELDS 5
static char *FLC_pfields[FLC_PFIELDS] = {
  "Calls", "Ops", "Ops/Call","wMops/Frame","wMops/Call"
};

#ifdef CT_INSTRUMENT
static char *FLC_pAccumfields[FLC_PFIELDS] = {
  "Calls", "AccumOps", "AccumOps/Call","wMops/Frame","wMops/Call"
};

#define FLC_PINDIV_FIELDS 5
static char *FLC_pIndivfields[FLC_PINDIV_FIELDS] = {
  "Calls", "IncOps", "IncOps/Call","ExcOps","ExcOps/Call"
};
#endif

/* print the header for the output */
static void FLC_Ops_header()
{
   char formatstring[10];
   int n;

   /* first, print a header for the tags, FLC_MAXTAGLEN long padded */
   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, "Function");

   /* now print the opcode names, padded to FLC_OPCODE_LEN */
   sprintf(formatstring, "%%%ds", FLC_OPSCNT_LEN);
   for (n = 0; n < FLC_PFIELDS; n++)
   {
      fprintf(FLC_output, formatstring, FLC_pfields[n]);
   }
   fputs("\n-----------\n", FLC_output);
}

#ifdef CT_INSTRUMENT
/* print the header for the output */
static void FLC_AccumOps_header()
{
   char formatstring[10];
   int n;

   /* first, print a header for the tags, FLC_MAXTAGLEN long padded */
   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, "Function");

   /* now print the opcode names, padded to FLC_OPCODE_LEN */
   sprintf(formatstring, "%%%ds", FLC_OPSCNT_LEN);
   for (n = 0; n < FLC_PFIELDS; n++)
   {
      fprintf(FLC_output, formatstring, FLC_pAccumfields[n]);
   }
   fputs("\n-----------\n", FLC_output);
}

/* print the header for the output */
static void FLC_IndivOps_header()
{
   char formatstring[10];
   int n;

   /* first, print a header for the tags, FLC_MAXTAGLEN long padded */
   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, "Function");

   /* now print the opcode names, padded to FLC_OPCODE_LEN */
   sprintf(formatstring, "%%%ds", FLC_OPSCNT_LEN);
   for (n = 0; n < FLC_PINDIV_FIELDS; n++)
   {
      fprintf(FLC_output, formatstring, FLC_pIndivfields[n]);
   }
   fputs("\n-----------\n", FLC_output);
}

static void FLC_IndivOpsWmops_header()
{
   char formatstring[10];
   int n;

   /* first, print a header for the tags, FLC_MAXTAGLEN long padded */
   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, "Function");

   /* now print the opcode names, padded to FLC_OPCODE_LEN */
   sprintf(formatstring, "%%%ds", FLC_OPSCNT_LEN);
   for (n = 0; n < FLC_PINDIV_FIELDS; n++)
   {
      fprintf(FLC_output, formatstring, FLC_pIndivfields[n]);
   }
   fputs("\n-----------\n", FLC_output);
}
#endif

static void FLC_Ops_print(char *prefix, FLC_Ops_Data *pL)
{
   char namestr[FLC_MAXTAGLEN + 1] = { 0 };
   char newprefix[FLC_MAXPREFIXLEN + 1] = { 0 };

   do
   {
      strncpy(namestr, prefix, FLC_MAXPREFIXLEN);
      strncat(namestr, pL->name, FLC_MAXTAGLEN - strlen(namestr));

      FLC_Ops_line(namestr, pL);

      /* check to see if there is a sublist */
      if (pL->subfirst != NULL)
      {
         strncpy(newprefix, prefix, FLC_MAXPREFIXLEN - 1);
         strcat(newprefix, "-");
         FLC_Ops_print(newprefix, pL->subfirst);
      }

      pL = pL->next;
   }
   while (pL != NULL);
}

#ifdef CT_INSTRUMENT
static void FLC_AccumOps_print(char *prefix, FLC_Ops_Data *pL)
{
   char namestr[FLC_MAXTAGLEN + 1] = { 0 };
   char newprefix[FLC_MAXPREFIXLEN + 1] = { 0 };

   do
   {
      strncpy(namestr, prefix, FLC_MAXPREFIXLEN);
      strncat(namestr, pL->name, FLC_MAXTAGLEN - strlen(namestr));

      FLC_AccumOps_line(namestr, pL);

      /* check to see if there is a sublist */
      if (pL->subfirst != NULL)
      {
         strncpy(newprefix, prefix, FLC_MAXPREFIXLEN - 1);
         strcat(newprefix, "-");
         FLC_AccumOps_print(newprefix, pL->subfirst);
      }

      pL = pL->next;
   }
   while (pL != NULL);
}

static void FLC_IndivOps_print(char *prefix, FLC_Ops_Data *pL)
{
   char namestr[FLC_MAXTAGLEN + 1] = { 0 };

   do
   {
      strncpy(namestr, prefix, FLC_MAXPREFIXLEN);
      strncat(namestr, pL->name, FLC_MAXTAGLEN - strlen(namestr));

      {
        char formatstring[10];

        /* first, print a header for the tags, FLC_MAXTAGLEN long padded */
        sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
        fprintf(FLC_output, formatstring, namestr);

        /* now print the opcode names, padded to FLC_OPCODE_LEN */
        sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
        fprintf(FLC_output, formatstring, pL->invocations);
        sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
        fprintf(FLC_output, formatstring, pL->fn_accum_total);
        sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
        fprintf(FLC_output, formatstring, pL->fn_accum_total/pL->invocations);
        sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
        fprintf(FLC_output, formatstring, pL->fn_total[0]);
        sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
        fprintf(FLC_output, formatstring, pL->fn_total[0]/pL->invocations);

        fputc('\n', FLC_output);
      }

      pL = pL->next;
   }
   while (pL != NULL);
}

static void FLC_IndivOpsWmops_print(char *prefix, FLC_Ops_Data *pL)
{
   char namestr[FLC_MAXTAGLEN + 1] = { 0 };

   do
   {
      strncpy(namestr, prefix, FLC_MAXPREFIXLEN);
      strncat(namestr, pL->name, FLC_MAXTAGLEN - strlen(namestr));

      {
        char formatstring[10];

        /* first, print a header for the tags, FLC_MAXTAGLEN long padded */
        sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
        fprintf(FLC_output, formatstring, namestr);

        /* now print the opcode names, padded to FLC_OPCODE_LEN */
        sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
        fprintf(FLC_output, formatstring, pL->invocations);
        sprintf(formatstring, "%%%dlf", FLC_OPSCNT_LEN);
        fprintf(FLC_output, formatstring, (float)pL->fn_accum_total / (float)ops_total->invocations /
                (1000.0f * (float)FLC_FRAMELEN));
        sprintf(formatstring, "%%%dlf", FLC_OPSCNT_LEN);
        if (pL->invocations < ops_total->invocations) {
          /* used for e.g. the function main() */
          fprintf(FLC_output, formatstring, (float)pL->fn_accum_total / (float)pL->invocations / (float)ops_total->invocations / 
                  (1000.0f * (float)FLC_FRAMELEN));
        } else {
          /* used for the most functions */
          fprintf(FLC_output, formatstring, (float)pL->fn_accum_total / (float)pL->invocations / 
                  (1000.0f * (float)FLC_FRAMELEN));
        }

        sprintf(formatstring, "%%%dlf", FLC_OPSCNT_LEN);
        fprintf(FLC_output, formatstring, (float)pL->fn_total[0] / (float)ops_total->invocations /
                (1000.0f * (float)FLC_FRAMELEN));

        sprintf(formatstring, "%%%dlf", FLC_OPSCNT_LEN);
        if (pL->invocations > ops_total->invocations) {
          /* used for the most functions */
          fprintf(FLC_output, formatstring, (float)pL->fn_total[0] / (float)pL->invocations /
                  (1000.0f * (float)FLC_FRAMELEN));
        } else {
          /* used for e.g. the function main() */
          fprintf(FLC_output, formatstring, (float)pL->fn_total[0] / (float)pL->invocations / (float)ops_total->invocations /
                  (1000.0f * (float)FLC_FRAMELEN));
        }

        fputc('\n', FLC_output);
      }

      pL = pL->next;
   }
   while (pL != NULL);
}
#endif

static void FLC_Ops_line(char *pre, FLC_Ops_Data *pL)
{
   char formatstring[10];

   /* first, print a header for the tags, FLC_MAXTAGLEN long padded */
   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, pre);

   /* now print the opcode names, padded to FLC_OPCODE_LEN */
   sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
   fprintf(FLC_output, formatstring, pL->invocations);
   sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
   fprintf(FLC_output, formatstring, pL->fn_total[0]);
   sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
   fprintf(FLC_output, formatstring, pL->fn_total[0]/pL->invocations);
   sprintf(formatstring, "%%%dlf", FLC_OPSCNT_LEN);
   fprintf(FLC_output, formatstring, (float)pL->fn_total[0]/(float)ops_total->invocations/(1000.0f * (float)FLC_FRAMELEN));
   sprintf(formatstring, "%%%dlf", FLC_OPSCNT_LEN);
   if (pL->invocations < ops_total->invocations) {
     /* used for e.g. the function main() */
     fprintf(FLC_output, formatstring, (float)pL->fn_total[0]/(float)pL->invocations/(float)ops_total->invocations/(1000.0f * (float)FLC_FRAMELEN));
   } else {
     /* used for the most functions */
     fprintf(FLC_output, formatstring, (float)pL->fn_total[0]/(float)pL->invocations/(1000.0f * (float)FLC_FRAMELEN));
   }

   fputc('\n', FLC_output);
}

#ifdef CT_INSTRUMENT
static void FLC_AccumOps_line(char *pre, FLC_Ops_Data *pL)
{
   char formatstring[10];

   /* first, print a header for the tags, FLC_MAXTAGLEN long padded */
   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, pre);

   /* now print the opcode names, padded to FLC_OPCODE_LEN */
   sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
   fprintf(FLC_output, formatstring, pL->invocations);
   sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
   fprintf(FLC_output, formatstring, pL->fn_accum_total);
   sprintf(formatstring, "%%%dld", FLC_OPSCNT_LEN);
   fprintf(FLC_output, formatstring, pL->fn_accum_total/pL->invocations);
   sprintf(formatstring, "%%%dlf", FLC_OPSCNT_LEN);
   fprintf(FLC_output, formatstring, pL->fn_accum_total/(float)ops_total->invocations/(float)FLC_FRAMELEN/1000.0f);
   sprintf(formatstring, "%%%dlf", FLC_OPSCNT_LEN);
   if (pL->invocations < ops_total->invocations) {
     /* used for e.g. the function main() */
     fprintf(FLC_output, formatstring, (float)pL->fn_accum_total/(float)pL->invocations/(float)ops_total->invocations/(1000.0f * (float)FLC_FRAMELEN));
   } else {
     /* used for the most functions */
     fprintf(FLC_output, formatstring, (float)pL->fn_accum_total/(float)pL->invocations/(1000.0f * (float)FLC_FRAMELEN));
   }

   fputc('\n', FLC_output);
}
#endif

/****** Memory usage printing *************/

static void FLC_Mem_print()
{
   FLC_Mem_Data *pL;
   FLC_Mem_Summaries *sums, *s_top;
   char formatstring[10];
   int n;

   s_top = NULL;
   memset(totals, 0, sizeof(totals));

   /* First compute sums per function */
   for (pL = mem_top; pL; pL = pL->next)
   {
      sums = FLC_Mem_find_function(s_top, pL->context);
      totals[pL->type] += pL->count;
      if (sums)
      {
         sums->counts[pL->type] += pL->count;
      }
      else
      {
         sums = (FLC_Mem_Summaries*) malloc(sizeof(struct FLC_Mem_Summaries));
         memset(sums, 0, sizeof(struct FLC_Mem_Summaries));
         sums->counts[pL->type] += pL->count;
         strncpy(sums->name, pL->context, FLC_MAXTAGLEN); sums->name[FLC_MAXTAGLEN] = '\0';
         sums->next = s_top;
         s_top = sums;
      }
   }


   /* Print summaries header ADD - DIV */

   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, "Function");
   sprintf(formatstring, "%%%ds", FLC_OPCODE_LEN);
   for (n = FLC_ADD; n <= FLC_DIV; n++)
   {
      fprintf(FLC_output, formatstring, Op_Names[n]);
   }

   fputs("\n-----------\n", FLC_output);

   for (sums = s_top; sums; sums = sums->next)
   {
      sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
      fprintf(FLC_output, formatstring, sums->name);
      sprintf(formatstring, "%%%dd", FLC_OPCODE_LEN);
      for (n = FLC_ADD; n <= FLC_DIV; n++)
      {
         fprintf(FLC_output, formatstring, sums->counts[n]);
      }
      fprintf(FLC_output, "\n");
   }

   fputs("\n-----------\n", FLC_output);

   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, "totals");
   sprintf(formatstring, "%%%dd", FLC_OPCODE_LEN);
   for (n = FLC_ADD; n <= FLC_DIV; n++)
   {
      fprintf(FLC_output, formatstring, totals[n]);
   }

   fputs("\n\n--------------------------------\n\n", FLC_output);

   /* Print summaries header SQRT - MISC */

   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, "Function");
   sprintf(formatstring, "%%%ds", FLC_OPCODE_LEN);
   for (n = FLC_SQRT; n <= FLC_MISC; n++)
   {
      fprintf(FLC_output, formatstring, Op_Names[n]);
   }

   fputs("\n-----------\n", FLC_output);

   for (sums = s_top; sums; sums = sums->next)
   {
      sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
      fprintf(FLC_output, formatstring, sums->name);
      sprintf(formatstring, "%%%dd", FLC_OPCODE_LEN);
      for (n = FLC_SQRT; n <= FLC_MISC; n++)
      {
         fprintf(FLC_output, formatstring, sums->counts[n]);
      }
      fprintf(FLC_output, "\n");
   }

   fputs("\n-----------\n", FLC_output);

   sprintf(formatstring, "%%-%ds", FLC_MAXTAGLEN);
   fprintf(FLC_output, formatstring, "totals");
   sprintf(formatstring, "%%%dd", FLC_OPCODE_LEN);
   for (n = FLC_SQRT; n <= FLC_MISC; n++)
   {
      fprintf(FLC_output, formatstring, totals[n]);
   }
}

static FLC_Mem_Summaries *FLC_Mem_find_function(FLC_Mem_Summaries *top,
                                                char *name)
{
   FLC_Mem_Summaries *pL;

   for (pL = top; pL; pL = pL->next)
      if (!strncmp(pL->name, name, FLC_MAXTAGLEN ))
         return pL;

   return pL;
}

static void FLC_print_sum()
{
   unsigned long gt, n;
   int i;

   fprintf(stdout, "\n\n--- SUMMARY ---\n\n");

   /* Print total ops */
   for (i=0; i<=ops_total->idxFnTotal; i++)
     fprintf(stdout, "Total Ops #%d: %lu\n", i, ops_total->fn_total[i]);
   

   /* Print overall total Program Memory Usage */
   gt = 0;
   for (n = 1; n < FLC_OPEND; n++)
      gt += totals[n] * Mem_Weights[n];

   fprintf(stdout, "Total Program ROM usage: %ld  (word)\n", gt);
}

static void FLC_print_frame_sum()
{
  int i;
  float tmp = 0.0f;

   fprintf(stdout, "\n--- Per Frame Summary ---\n");

   for (i=0; i<=ops_total->idxFnTotal; i++) {
     tmp += (ops_total->fn_total[i] / (float) ops_total->invocations);
   }

   fprintf(stdout,
           "Number of Frames: %ld \nAverage Ops/frame: %8.2f   Max Ops/frame: %ld\n\n",
           ops_total->invocations, tmp, ops_total->fn_max);

   fprintf(stdout, "Maximum frame WMOPS: %f   (Frame is %4.2f ms)\n",
           ops_total->fn_max / (1000.0f * (float) FLC_FRAMELEN), FLC_FRAMELEN);

   fprintf(stdout, "Average frame WMOPS: %f\n",
           tmp / (1000.0f * (float)FLC_FRAMELEN) );
}

#endif
