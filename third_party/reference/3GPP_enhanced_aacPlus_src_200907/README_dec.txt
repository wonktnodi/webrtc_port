===================================================================

                3GPP Enhanced aacPlus decoder
                           Linux/Win32
                       for Intel x86 CPUs
 
===================================================================

CONTENTS: Enhanced aacPlus decoder source code


REVISION NOTES:

 - Contents:

 FloatFR_aacPlusdec/

   Makefile               -- Linux Makefile
   FloatFR_aacPlusdec_mpeg4.dsw -- Win32 MSVC 6.0 workspace
   FloatFR_aacPlusdec_mpeg4.dsp -- Win32 MSVC 6.0 makefile

   src/                   -- directory for the frontend
   FloatFR_bitbuflib/     -- bitstream reading/writing library
   FloatFR_aacdec/        -- AAC decoder library
   FloatFR_sbrdeclib/     -- SBR decoder library
   FloatFRlib/            -- general purpose functionalities
 
   lib/                   -- precompiled MPEG-4 / 3GPP file format 
      w32/                   and audio file library (for wav support),
      linux/                 for Linux OS (requires gcc 3.3) and 
                             Windows 32-bit


 - Compilation: 

   o for Win32 use the MSVC FloatFR_aacPlusdec_mpeg4.dsw 6.0
     workspace, activate the FloatFR_aacPlusdec project and
     choose the appropriate "Win32 CT Release/Debug" target

   o for Linux use >gmake clean all< in the FloatFR_aacPlusdec/ 
     root directory


 - MONO_ONLY, LP_SBR_ONLY

   These defines are introduced to be able to compile a mono-only
   decoder using LP-SBR. It is set by default when choosing the 
   Mono compile targets in the Win32 workspace, resp. setting the 
   MONO=1 option on the Linux gmake commandline.


===================================================================
for info mailto: 3gpp-support@codingtechnologies.com
===================================================================
