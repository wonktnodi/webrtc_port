===================================================================

                 3GPP Enhanced aacPlus encoder
                           Linux/Win32
                       for Intel x86 CPUs

===================================================================

 
CONTENTS: Enhanced aacPlus encoder source code


REVISION NOTES:

 - Contents:

 FloatFR_aacPlusenc/

   Makefile               -- Linux Makefile
   FloatFR_aacPlusEnc.dsw -- Win32 MSVC 6.0 workspace
   FloatFR_aacPlusEnc.dsp -- Win32 MSVC 6.0 makefile

   src/                   -- directory for the frontend
   FloatFR_bitbuflib/     -- bitstream reading/writing library
   FloatFR_fastaacenc/    -- AAC encoder library
   FloatFR_resamplib/     -- resampler library
   FloatFR_sbrenclib/     -- SBR encoder library
   FloatFRlib/            -- general purpose functionalities
 
   lib/                   -- precompiled MPEG-4 / 3GPP file format 
      w32/                   and audio file library (for wav support),
      linux/                 for Linux OS (requires gcc 3.3) and 
                             Windows 32-bit

 - Compilation: 

   o for Win32 use the FloatFR_aacPlusEnc.dsw MSVC 6.0
     workspace, activate the FloatFR_aacPlusenc project and
     choose the appropriate "Win32 CT Release/Debug" target

   o for Linux use >gmake clean all< in the FloatFR_aacPlusenc/
     root directory


 - MONO_ONLY

   This define is introduced to be able to compile a mono-only
   encoder. It is set by default when choosing the Mono compile
   targets in the Win32 workspace, resp. setting the MONO=1 option 
   on the Linux gmake commandline. Note that for the mono 
   executable only mono input wave files are supported.


 - bitrate switching simulation encoder frontend

   If you are interested in an encoder frontend which simulates
   bitrate switching you can use the brswitchmain.c file instead 
   of the main.c file to obtain such an executable.
   For Win32 you may disable the main.c file and enable the 
   brswitchmain.c file instead within your MSVC environment. 
   For Linux this alternative executable is included as a second 
   target and built automatically if the "all" target is select.


===================================================================
for info mailto: 3gpp-support@codingtechnologies.com
===================================================================
