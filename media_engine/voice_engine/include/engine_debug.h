#ifndef __MEDIA_ENGINE_DEBUG_H__
#define __MEDIA_ENGINE_DEBUG_H__

typedef struct media_engine_debuger media_engine_debuger;

#ifdef __cplusplus
extern "C" {
#endif 

int media_engine_debug_strat_rtp_dump(media_engine_debuger*, int, const char*, int);
int media_engine_debug_stop_rtp_dump(media_engine_debuger*, int, int);
int media_engine_debug_rtp_dump_active(media_engine_debuger*, int, int);

int media_engine_debug_write_rtp_dump_extern_info(media_engine_debuger*,
  const char*, const char*, int);

#ifdef __cplusplus
}
#endif 


#endif // __MEDIA_ENGINE_DEBUG_H__
