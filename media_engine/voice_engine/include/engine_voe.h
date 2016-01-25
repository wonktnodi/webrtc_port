#ifndef __MEDIA_ENGINE_VOE_H__
#define __MEDIA_ENGINE_VOE_H__

#include "common_types.h"

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct media_engine_voice_base media_engine_voice_base;

int media_engine_voe_create_channel(media_engine_voice_base *);
int media_engine_voe_delete_channel(media_engine_voice_base *, int);

int media_engine_voe_start_send(media_engine_voice_base *, int);
int media_engine_voe_stop_send(media_engine_voice_base *, int);
int media_engine_voe_send_status(media_engine_voice_base *, int, bool&);

int media_engine_voe_start_receive(media_engine_voice_base *, int);
int media_engine_voe_stop_receive(media_engine_voice_base *, int);

int media_engine_voe_start_playout(media_engine_voice_base *, int);
int media_engine_voe_stop_playout(media_engine_voice_base *, int);

int media_engine_voe_get_delay_with_playouttimestamp(media_engine_voice_base*, int, uint32_t, uint32_t&);

int media_engine_voe_get_network_statistics(media_engine_voice_base*, int, webrtc::NetworkStatistics&);

int media_engine_voe_set_hold_status(media_engine_voice_base*, int, bool, int);
int media_engine_voe_get_hold_status(media_engine_voice_base*, int, bool&, int&);

int media_engine_voe_get_last_error(media_engine_voice_base*);

int media_engine_voe_enable_data_combine(media_engine_voice_base *, int, bool , uint32_t);
int media_engine_voe_enable_data_separate(media_engine_voice_base *, int, bool);

#ifdef __cplusplus
}
#endif 




#endif // __MEDIA_ENGINE_VOE_H__
