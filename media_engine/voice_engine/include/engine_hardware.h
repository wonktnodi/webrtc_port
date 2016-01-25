#ifndef __MEDIA_ENGINE_HARDWARE_H__
#define __MEDIA_ENGINE_HARDWARE_H__

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct media_engine_hardware media_engine_hardware;

int engine_hardware_get_num_record_dev(media_engine_hardware *, int&);
int engine_hardware_get_num_play_dev(media_engine_hardware *, int&);

int engine_hardware_get_record_dev_name(media_engine_hardware*, int index,
  char strNameUTF8[128], char strGuidUTF8[128]);

int engine_hardware_get_play_dev_name(media_engine_hardware*, int index,
  char strNameUTF8[128], char strGuidUTF8[128]);

int engine_hardware_set_record_dev(media_engine_hardware *, int, int);
int engine_hardware_set_playout_dev(media_engine_hardware *, int);

int engine_hardware_get_cur_record_dev(media_engine_hardware *, int&);
int engine_hardware_get_cur_playout_dev(media_engine_hardware *, int&);

#ifdef __cplusplus
}
#endif

#endif // __MEDIA_ENGINE_HARDWARE_H__
