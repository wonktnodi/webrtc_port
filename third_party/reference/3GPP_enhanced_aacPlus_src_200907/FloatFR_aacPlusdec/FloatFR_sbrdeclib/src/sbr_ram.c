/*
  Memory layout
  This module declares all static and dynamic memory spaces
*/
#include <stdio.h>
#include "sbrdecoder.h"
#include "sbr_ram.h"
#include "FloatFR.h"

extern float WorkBufferCore[];

/*
  \name StaticSbrData

  Static memory areas, must not be overwritten in other sections of the decoder
*/
/* @{ */

/*  Filter states for QMF-analysis. <br>
  Dimension: #MAXNRQMFCHANNELS * #QMF_FILTER_STATE_ANA_SIZE */
float sbr_CodecQmfStatesAnalysis[MAXNRQMFCHANNELS * QMF_FILTER_STATE_ANA_SIZE];

/*  Filter states for QMF-synthesis.  <br>
  Dimension: #MAXNRQMFCHANNELS * #QMF_FILTER_STATE_SYN_SIZE */
float sbr_QmfStatesSynthesis[MAXNRQMFCHANNELS * QMF_FILTER_STATE_SYN_SIZE];

/*  Gains (matissas) of last envelope, needed for smoothing.  <br>
  Dimension: #MAXNRSBRCHANNELS x #MAX_FREQ_COEFFS */
float sbr_GainSmooth[MAXNRSBRCHANNELS][MAX_FREQ_COEFFS];

/*  Noise levels of last envelope, needed for smoothing.  <br>
  Dimension: #MAXNRSBRCHANNELS x #MAX_FREQ_COEFFS */
float sbr_NoiseSmooth[MAXNRSBRCHANNELS][MAX_FREQ_COEFFS];

/*  Frequency tables derived from the SBR-header */
FREQ_BAND_DATA FreqBandData;

/*  Delayed spectral data needed for the dynamic framing of SBR */
#ifdef LP_SBR_ONLY
float sbr_OverlapBuffer[MAXNRQMFCHANNELS][MAX_OV_COLS * NO_SYNTHESIS_CHANNELS];
#else
float sbr_OverlapBuffer[MAXNRQMFCHANNELS][2 * MAX_OV_COLS * NO_SYNTHESIS_CHANNELS];
#endif


/*  Filter states (real part) for LPC used in the LPP-transposer */
float sbr_LpcFilterStatesReal[MAXNRQMFCHANNELS][LPC_ORDER][NO_ANALYSIS_CHANNELS];

#ifndef LP_SBR_ONLY
/*  Filter states (imaginary part) for LPC used in the LPP-transposer */
float sbr_LpcFilterStatesImag[MAXNRQMFCHANNELS][LPC_ORDER][NO_ANALYSIS_CHANNELS];
#endif

/*  Transposer settings derived from the SBR-header */
TRANSPOSER_SETTINGS sbr_TransposerSettings;

/*  Some data from the previous frame needed for delta-coding over time
  and for concealment. */
SBR_PREV_FRAME_DATA PrevFrameData[MAXNRSBRCHANNELS];

/*  Bitstream data for the previous frame needed for concealment. */
SBRBITSTREAM sbr_PrevBitstream;




/* @} */


/*
  \name DynamicSbrData

  Dynamic memory areas, might be reused in other algorithm sections,
  e.g. the core decoder
*/
/* @{ */

#ifdef LP_SBR_ONLY
/*  This buffer stores half of the reconstructed left time data signal
  until the right channel is completely finished */
float *InterimResult = WorkBufferCore;
#else
/*  The work buffer #WorkBufferCore of the aac-core (see aac_ram.cpp)
  will be reused as #WorkBuffer2 in the SBR part. Minimum size of
  #WorkBufferCore must be #MAX_COLS * #NO_SYNTHESIS_CHANNELS.
  #WorkBuffer2 is the second half of the SBR work buffer. */
float *WorkBuffer2 = WorkBufferCore;

/*  This buffer stores half of the reconstructed left time data signal
  until the right channel is completely finished */
float InterimResult[MAX_FRAME_SIZE];
/* @} */
#endif /* LP_SBR_ONLY */

