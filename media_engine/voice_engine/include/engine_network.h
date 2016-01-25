#ifndef __MEDIA_ENGINE_NETWORK_H__
#define __MEDIA_ENGINE_NETWORK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct media_engine_network media_engine_network;

typedef struct engine_transport {  
  int (*outgo_audio_data)(int, uint8_t, uint8_t, uint16_t,uint32_t, uint32_t, const char *, int);
} engine_transport;


int media_engine_network_register_transport(media_engine_network*, engine_transport*);

int media_engine_network_income_rtp_data(media_engine_network*, int, const char*, int);
int media_engine_network_income_rtp_raw_data(media_engine_network*, int, uint8_t bit_mask,
  uint8_t pt, uint16_t seq, uint32_t tm, uint32_t ssrc, const char * data, int len);
int media_engine_network_income_rtcp_data(media_engine_network*, int, const char*, int);

#ifdef __cplusplus
}
#endif


#endif // __MEDIA_ENGINE_NETWORK_H__
