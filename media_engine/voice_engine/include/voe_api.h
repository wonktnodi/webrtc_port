#ifndef __MEDIAENGINE_INCLUDE_VOE_API_H__
#define __MEDIAENGINE_INCLUDE_VOE_API_H__

#include "common_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// initialize
int voe_init(const char* file_name_utf8, const unsigned int trace_filter,
             const unsigned int trace_module);
int voe_release();

// log interfaces.
int voe_set_log_path(const char* fileNameUTF8,
                     const bool addFileCounter = false);
int voe_set_log_filter(const unsigned int filter, const unsigned int module);

// device interfaces
int voe_get_play_device_count(int &dev_num);
int voe_get_play_device_name(int idx, char strNameUTF8[128],
                             char strGuidUTF8[128]);
int voe_set_play_device(int idx);
int voe_get_play_device_volume(unsigned int &vol);
int voe_set_play_device_volume(unsigned int vol);

int voe_get_rec_device_count(int &dev_num);
int voe_get_rec_device_name(int idx, char strNameUTF8[128],
                            char strGuidUTF8[128]);
int voe_set_rec_device(int idx);
int voe_get_rec_device_volume(unsigned int &vol);
int voe_set_rec_device_volume(unsigned int vol);

int voe_register_audio_transport(int (*outgo_audio_data)(int, uint8_t, uint8_t,
                                 uint16_t, uint32_t, uint32_t, const char *, int));
int voe_unregister_audio_transport();
int voe_set_transport_data(int chl, uint8_t bit_mask,
                           uint8_t pt, uint16_t seq, uint32_t tm, uint32_t ssrc, const char * data,
                           int len);

// codec interfaces
int voe_get_audio_codec_count(int &num);
int voe_get_audio_codec_detail(int idx, webrtc::CodecInst &inst);

// APM interfaces.
int voe_get_agc_mode(bool &enable, int &mode);
int voe_set_agc_mode(bool enable, int mode);
int voe_set_rx_agc_mode(int chl, bool enable, int mode);
int voe_get_rx_agc_mode(int chl, bool &enable, int &mode);

int voe_get_ns_mode(bool &enable, int &mode);
int voe_set_ns_mode(bool enable, int mode = 0);
int voe_get_rx_ns_mode(int chl, bool &enable, int &mode);
int voe_set_rx_ns_mode(int chl, bool enable, int mode);

int voe_get_ec_mode(bool &enable, int &mode);
int voe_set_ec_mode(bool enable, int mode = 0);

int voe_set_aecm_mode(bool enable, int mode);
int voe_get_aecm_mode(bool &enable, int &mode);

// channel interfaces.
int voe_channel_set_codec(int chl, int payload_type);
int voe_channel_get_codec(int chl, int &payload_type);

int voe_create_channel();
int voe_delete_channel(int chl);

int voe_channel_start_send(int chl);
int voe_channel_stop_send(int chl);
int voe_channel_send_status(int chl, bool &status);

int voe_channel_start_receive(int chl);
int voe_channel_stop_receive(int chl);

int voe_channel_start_playout(int chl);
int voe_channel_stop_playout(int chl);

int voe_channel_enable_packet_combine(int chl, bool enable, int num_combined);
int voe_channel_enable_packet_separate(int chl, bool enable);

int voe_channel_set_vad(int chl, bool enable, int mode, bool dtx_disable);
int voe_channel_get_vad(int chl, bool &enable, int &mode, bool &dtx_disable);

int voe_set_input_mute();
int voe_get_input_mute();

int voe_channel_vad_detecting();
int voe_channel_vad_status();

int voe_channel_hold(int chl, bool hold_flag);
int voe_channel_hold_status(int chl, bool &hold_flag);

int voe_channel_get_delay_estimate();	

int voe_get_speech_input_level();
int voe_get_speech_output_level(int chl);

// voe file interfaces
int voe_start_record_mircophone(const char* file_name_utf8,
                                int compress_type = 0, int max_size = -1);
int voe_stop_record_mircophone();
int voe_channel_start_play_file_locally(int channel,
  const char *file_name_utf8, bool loop, int format = 1, float volume_scale = 1.0f);
int voe_channel_stop_play_file_locally(int channel);

int voe_channel_start_record_playout();
int voe_channel_stop_record_playout();

int voe_channel_start_play_file_as_mic(int channel, const char* mic_file_utf8, bool loop,
  bool mix_with_mic, int format, float volume_scale  = 1.0f);
int voe_channel_stop_play_file_as_mic(int channel);
int voe_channel_set_mic_file_scale();
int voe_channel_is_play_file_as_mic();

// debug interfaces
int voe_debug_start_data_dump(int channel, const char* utf8_name, int direction);
int voe_debug_stop_data_dump(int channel, int direction);
int voe_debug_data_dump_actived(int channel, int direction, bool &enable);
int voe_debug_write_dump_date_externinfo(const char* utf8_file_name,
  const char* buf, int buf_len);

int voe_last_error();

#ifdef __cplusplus
}
#endif


#endif // __MEDIAENGINE_INCLUDE_VOE_API_H__
