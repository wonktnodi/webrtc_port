#ifndef __MEDIA_ENGINE_AUDIO_VOLUME_CONTROL_H__
#define __MEDIA_ENGINE_AUDIO_VOLUME_CONTROL_H__

typedef struct media_engine_voice_volume media_engine_voice_volume;

#ifdef __cplusplus
extern "C" {
#endif 

int media_engine_get_speaker_volume(media_engine_voice_volume*, unsigned int&);
int media_engine_set_speaker_volume(media_engine_voice_volume*, unsigned int);

int media_engine_set_system_output_mute(media_engine_voice_volume*, bool);
int media_engine_get_system_output_mute(media_engine_voice_volume*, bool&);

int media_engine_set_mic_volume(media_engine_voice_volume*, unsigned int);
int media_engine_get_mic_volume(media_engine_voice_volume*, unsigned int&);

int media_engine_set_input_mute(media_engine_voice_volume*, int, bool);
int media_engine_get_input_mute(media_engine_voice_volume*, int, bool&);

int media_engine_set_system_input_mute(media_engine_voice_volume*, bool);
int media_engine_get_system_input_mute(media_engine_voice_volume*, bool&);

int media_engine_get_speech_input_level(media_engine_voice_volume*, unsigned int&);
int media_engine_get_speech_output_level(media_engine_voice_volume*, int, unsigned int&);

int media_engine_get_speech_input_level_range(media_engine_voice_volume*,unsigned int&);
int media_engine_get_speech_output_level_range(media_engine_voice_volume*, int, unsigned int&);

int media_engine_set_channel_output_volume_scaling(media_engine_voice_volume*, int, float);
int media_engine_get_channel_output_volume_scaling(media_engine_voice_volume*, int, float&);

int meida_engine_set_output_volume_pan(media_engine_voice_volume*, int, float, float);
int meida_engine_get_output_volume_pan(media_engine_voice_volume*, int, float &, float&);


#ifdef __cplusplus
}
#endif

#endif // __MEDIA_ENGINE_AUDIO_VOLUME_CONTROL_H__
