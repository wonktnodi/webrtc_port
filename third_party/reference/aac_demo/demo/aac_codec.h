#ifndef __AUDIO_CODEC_AAC_CODEC_H__
#define __AUDIO_CODEC_AAC_CODEC_H__


struct aac_encoder_t;

int open_aac_encoder(struct aac_encoder_t *&enc, int sample_rate, int bitrate,
  int chl_in, int chl_out, int bandwidth = 0);
int close_aac_encoder(struct aac_encoder_t *enc);

int aac_encode(struct aac_encoder_t *enc);



#endif // __AUDIO_CODEC_AAC_CODEC_H__
