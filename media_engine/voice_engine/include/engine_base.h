#ifndef __MEDIA_ENGINE_BASE_H__
#define __MEDIA_ENGINE_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct media_engine media_engine;
typedef struct media_engine_hardware media_engine_hardware;
typedef struct media_engine_voice_base media_engine_voice_base;
typedef struct media_engine_voice_codec media_engine_voice_codec;
typedef struct media_engine_network media_engine_network;
typedef struct media_engine_voice_file media_engine_voice_file;
typedef struct media_engine_apm media_engine_apm;
typedef struct media_engine_voice_volume media_engine_voice_volume;
typedef struct media_engine_debuger media_engine_debuger;

// Specifies the amount and type of trace information which will be
// created by the VoiceEngine.
int media_engine_set_trace_filter(const unsigned int filter, const unsigned int module);

// Sets the name of the trace file and enables non-encrypted trace messages.
int media_engine_set_trace_file(const char* fileNameUTF8, const bool addFileCounter = false);

int media_engine_create(media_engine**);

int media_engine_delete(media_engine*&);

int media_engine_addref(media_engine*);

int media_engine_release(media_engine*);

// Initiates all common parts of the MediaEngine; e.g. all
// encoders/decoders, the sound card and core receiving components.
int media_engine_init(media_engine*);

// Terminates all MediaEngine functions and releases allocated resources.
int media_engine_terminate(media_engine*);

int media_engine_set_speaker_mute(media_engine*, bool);
int media_engine_set_speaker_vol(media_engine*, int);

int media_engine_set_mic_mute(media_engine*, bool);
int media_engine_set_mic_vol(media_engine*, int);

int media_engine_get_hardware(media_engine* ,media_engine_hardware*&);

int media_engine_release_hardware(media_engine_hardware*&);

int media_engine_get_voe_base(media_engine*, media_engine_voice_base *&);

int media_engine_release_voe_base(media_engine_voice_base*&);

int media_engine_get_voe_codec(media_engine*, media_engine_voice_codec *&);

int media_engine_release_voe_codec(media_engine_voice_codec *&);

int media_engine_get_network(media_engine*, media_engine_network*&);

int media_engine_release_network(media_engine_network*&);

int media_engine_get_voe_file(media_engine*, media_engine_voice_file*&);

int media_engine_release_voe_file(media_engine_voice_file*&);

int media_engine_get_apm(media_engine*, media_engine_apm*&);

int media_engine_release_apm(media_engine_apm*&);

int media_engine_get_voice_volume(media_engine*, media_engine_voice_volume*&);

int media_engine_release_voice_volume(media_engine_voice_volume*&);

int media_engine_get_debuger(media_engine*, media_engine_debuger *&);

int media_engine_release_debuger(media_engine_debuger *&);

#ifdef __cplusplus
}
#endif


#endif // __MEDIA_ENGINE_BASE_H__
