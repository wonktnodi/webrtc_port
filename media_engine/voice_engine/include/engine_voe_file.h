#ifndef __MEDIA_ENGINE_VOE_FILE_H__
#define __MEDIA_ENGINE_VOE_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct media_engine_voice_file media_engine_voice_file;

int media_engine_voice_start_play_file_as_mic(media_engine_voice_file* file,
  int channel, const char* mic_file_utf8, bool loop, bool mix_with_mic,
  int format, float volume_scale  = 1.0f );
int media_engine_voice_stop_play_file_as_mic(media_engine_voice_file* file, int channel);
int media_engine_voice_set_mic_file_scale(media_engine_voice_file* file, int channel, int scale);
int media_engine_voice_is_play_file_as_mic(media_engine_voice_file* file, int channel);

int media_engine_voice_start_play_file_locally(media_engine_voice_file* file, int channel,
  const char *file_name_utf8, bool loop, int format = 1, float volume_scale = 1.0f);
int media_engine_voice_stop_play_file_locally(media_engine_voice_file* file, int channel);
int media_engine_voice_is_play_file_locally(media_engine_voice_file *file, int channel);

int media_engine_voice_start_record_playout(media_engine_voice_file* file,
  int channel, const char* file_name_utf8);
int media_engine_voice_stop_record_playout(media_engine_voice_file* file, int channel);

int media_engine_voice_start_record_mircophone(media_engine_voice_file* file,
  const char* file_name_utf8, int compress_type, int max_size);
int media_engine_voice_stop_record_mircophone(media_engine_voice_file* file);


#ifdef __cplusplus
}
#endif 

#endif // __MEDIA_ENGINE_VOE_FILE_H__
