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

 $Id: FFR_aacPLUScheck.c,v 1.1.2.6 2004/02/17 15:36:16 jr Exp $

*******************************************************************************/
/*
  \file
  \brief  Payload Extraction $Revision: 1.1.2.6 $
  \author Thomas Ziegler
*/

#include "FloatFR.h"
#include "sbrdecoder.h"
#include "FFR_bitbuffer.h"
#include "FFR_aacPLUScheck.h"

#include "flc.h" /* the 3GPP instrumenting tool */

#define aacPRO_MAXPAYLOAD 512

#ifndef SBR_EXTENSION_MPEG
#define SBR_EXTENSION_MPEG SBR_EXTENSION
#endif

#ifndef SBR_EXTENSION_CRC_MPEG
#define SBR_EXTENSION_CRC_MPEG SBR_EXTENSION_CRC
#endif


/*
  \brief Extraction of aacPlus - specific payload from fill element

  \return void
*/

void FFRaacplus_checkForPayload(HANDLE_BIT_BUF bs,
                                SBRBITSTREAM *streamSBR,
                                int prev_element)
{
  int i;
  int count=0;
  int esc_count=0;

  FLC_sub_start("FFRaacplus_checkForPayload");

  MOVE(2); /* counting previous operations */


  FUNC(2);
  count = ReadBits(bs,4);

  ADD(1); BRANCH(1);
  if (count == 15)
  {
    FUNC(2);
    esc_count = ReadBits(bs,8);

    ADD(1);
    count =  esc_count + 14;
  }

  BRANCH(1);
  if (count)
  {
     unsigned char extension_type;

    FUNC(2);
    extension_type = (unsigned char) ReadBits(bs,4);


    ADD(8); LOGIC(7); BRANCH(1);
    if (   (prev_element == SBR_ID_SCE || prev_element == SBR_ID_CPE)
        && ((extension_type == SBR_EXTENSION) || (extension_type == SBR_EXTENSION_CRC) || (extension_type == SBR_EXTENSION_MPEG) || (extension_type == SBR_EXTENSION_CRC_MPEG))
        && (count < MAXSBRBYTES) && (streamSBR->NrElements < MAXNRELEMENTS)    )
    {
      FUNC(2); INDIRECT(1); STORE(1);
      streamSBR->sbrElement [streamSBR->NrElements].Data[0] = (unsigned char) ReadBits(bs,4);

      PTR_INIT(1); /* streamSBR->sbrElement [streamSBR->NrElements].Data[i] */
      LOOP(1);
      for (i=1; i<count; i++)
      {
        FUNC(2); STORE(1);
        streamSBR->sbrElement [streamSBR->NrElements].Data[i] = (unsigned char) ReadBits(bs,8);
      }

      MOVE(2);
      streamSBR->sbrElement[streamSBR->NrElements].ExtensionType = extension_type;
      streamSBR->sbrElement[streamSBR->NrElements].Payload = count;

      ADD(1); INDIRECT(1); STORE(1);
      streamSBR->NrElements += 1;
    }
    else
    {
      FUNC(2);
      ReadBits(bs,4);

      LOOP(1);
      for (i=1; i<count; i++)
      {
        FUNC(2);
        ReadBits(bs,8);
      }
    }
  }

  FLC_sub_end();
}


