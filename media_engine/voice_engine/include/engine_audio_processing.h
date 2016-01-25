#ifndef __MEDIA_ENGINE_AUDIO_PROCESSING_H__
#define __MEDIA_ENGINE_AUDIO_PROCESSING_H__

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct media_engine_apm media_engine_apm;

int media_engine_apm_set_ns_status(media_engine_apm *, bool, int = 0);
int media_engine_apm_get_ns_status(media_engine_apm *, bool&, int&);

int media_engine_apm_set_rx_ns_status(media_engine_apm *, int, bool, int = 0);
int media_engine_apm_get_rx_ns_status(media_engine_apm *, int, bool&, int&);

int media_engine_apm_set_agc_status(media_engine_apm *, bool, int = 0);
int media_engine_apm_get_agc_status(media_engine_apm *, bool&, int&);

int media_engine_apm_set_rx_agc_status(media_engine_apm *, int, bool, int = 0);
int media_engine_apm_get_rx_agc_status(media_engine_apm *, int, bool&, int&);

int media_engine_apm_set_ec_status(media_engine_apm *, bool, int = 0);
int media_engine_apm_get_ec_status(media_engine_apm *, bool&, int&);

int media_engine_apm_set_aecm_mode(media_engine_apm *, int = 3, bool = true);
int media_engine_apm_get_aecm_mode(media_engine_apm *, int&, bool&);

int media_engine_apm_enable_hpf(media_engine_apm*, bool);
int media_engine_apm_is_hpf_enabled(media_engine_apm *, bool&);

int media_engine_apm_set_vad_status(media_engine_apm*, int, bool, int, bool);
int media_engine_apm_get_vad_status(media_engine_apm *, int, bool&, int&, bool&);

#ifdef __cplusplus
}
#endif



#endif // __MEDIA_ENGINE_AUDIO_PROCESSING_H__
