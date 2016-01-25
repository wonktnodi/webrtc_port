// aac_test.cpp : Defines the entry point for the console application.
//
#include <assert.h>
#include <stdlib.h>

#include "lib/au_channel.h"
#include "mp4file.h"
#include "../src/aacenc.h"
#include "../src/HEAAC_ENC.h"
#include "../src/HEAAC_DEC.h"

int main(int argc, char* argv[]) {
  HANDLE_MP4_FILE hMp4File = 0;
  FILE *pcm_fp = 0;

  hAudioChannel inputFile = 0;
  AuChanInfo inputInfo;
  AuChanType auType = TYPE_AUTODETECT ; /* must be set */
  AuChanMode auFlags = AU_CHAN_READ;
  short TimeDataPcm[AACENC_BLOCKSIZE * 2 * MAX_CHANNELS];

  inputInfo.bitsPerSample = 16 ;       /* only relevant if valid == 1 */
  inputInfo.sampleRate    = 44100 ;    /* only relevant if valid == 1 */
  inputInfo.nChannels      = 2 ;        /* only relevant if valid == 1 */
  inputInfo.nSamples       = 0 ;        /* only relevant if valid == 1 */
  inputInfo.isLittleEndian = 1;

  inputInfo.fpScaleFactor  = AACENC_PCM_LEVEL ; /* must be set */
  inputInfo.valid          = 1 ;        /* must be set */
  inputInfo.useWaveExt     = 0;

  pcm_fp = fopen("f:/temp/im/aac_pcm.pcm", "wb");
  int ret = AuChannelOpen (&inputFile, "c:\\audio\\ave_maria_48.wav", auFlags,
                           &auType, &inputInfo);

  IHEAAC_ENC* enc = CreateAACEnc();
  assert(enc && enc->Init(inputInfo.sampleRate, inputInfo.nChannels, 32000));

  IHEAAC_DEC* dec = CreateAACDec();
  assert(dec && dec->Init(inputInfo.sampleRate, 26000));
  
  int inSamples = AACENC_BLOCKSIZE * inputInfo.nChannels;
  int numSamplesRead = 0;

  inSamples = enc->GetSampleCount();

  std::string xxx, out_pcm;
  while (1) {
    if (AuChannelReadShort(inputFile, TimeDataPcm, inSamples,
                           &numSamplesRead) != AU_CHAN_OK) {
      fprintf(stderr, "failed to read source file!\n");
      return 0;
    }

    enc->Enc(TimeDataPcm, xxx);
    dec->Dec(xxx, out_pcm, true);
    
    fwrite(out_pcm.c_str(), 1, out_pcm.length(), pcm_fp);
  }



  if (enc)
    DestroyAACEnc(enc);

  if (inputFile)
    AuChannelClose(inputFile);

  if (pcm_fp)
    fclose(pcm_fp);

  system("pause");
  return 0;
}

