#include "voe_api.h"
#include <windows.h>

#include "engine_base.h"
#include "engine_network.h"
#include "engine_hardware.h"
#include "common_types.h"
#include "engine_voe_codec.h"
#include "engine_audio_volume.h"
#include "engine_voe.h"
#include "engine_audio_processing.h"
#include "engine_voe_file.h"
#include "engine_debug.h"

#ifdef __cplusplus
#define  EXTERNC  extern "C"
#else
#define  EXTERNC
#endif

using namespace webrtc;

static bool s_voe_initializing = false;
static bool s_voe_initialized = false;
static unsigned long s_voe_ref = 0;  // reference count for voice engine.

static engine_transport s_transport = {0};

static media_engine *_eng_ptr = 0;
static media_engine_voice_base *_eng_base = 0;
static media_engine_network *_eng_network_ptr = 0;
static media_engine_voice_file *_voice_file = 0;
static media_engine_apm *_eng_apm = 0;
static media_engine_voice_volume *_voice_volume = 0;
static media_engine_debuger *_eng_dbg = 0;
static media_engine_hardware *_eng_hardware = 0;
static media_engine_voice_codec *_audio_codec = 0;
static engine_transport _audio_transport = {0};


EXTERNC int voe_set_log_path(const char* fileNameUTF8,
                             const bool addFileCounter /* = false */) {
  return media_engine_set_trace_file(fileNameUTF8, addFileCounter);
}

EXTERNC int voe_set_log_filter(const unsigned int filter,
                               const unsigned int module) {
  return media_engine_set_trace_filter(filter, module);
}

EXTERNC int  voe_init(const char* file_name_utf8,
                      const unsigned int trace_filter,
                      const unsigned int trace_module) {
  if (s_voe_initializing)
    return -1;

  if (s_voe_initialized) {
    InterlockedIncrement(&s_voe_ref);
    return 0;
  }

  s_voe_initializing = true;

  int ret = media_engine_create(&_eng_ptr);
  
  if (ret) goto cleanup;

  ret = voe_set_log_filter(trace_filter, trace_module);

  if (file_name_utf8)
    ret = voe_set_log_path(file_name_utf8);

  ret = media_engine_init(_eng_ptr);

  if (ret) goto cleanup;

  ret = media_engine_get_voe_base(_eng_ptr, _eng_base);

  if (ret) goto cleanup;

  ret = media_engine_get_network(_eng_ptr, _eng_network_ptr);

  if (ret) goto cleanup;

  ret = media_engine_get_voe_file(_eng_ptr, _voice_file);

  if (ret) goto cleanup;

  ret = media_engine_get_apm(_eng_ptr, _eng_apm);

  if (ret) goto cleanup;

  ret = media_engine_get_voice_volume(_eng_ptr, _voice_volume);

  if (ret) goto cleanup;

  ret = media_engine_get_debuger(_eng_ptr, _eng_dbg);

  if (ret) goto cleanup;

  ret = media_engine_get_hardware(_eng_ptr, _eng_hardware);

  if (ret) goto cleanup;

  ret = media_engine_get_voe_codec(_eng_ptr, _audio_codec);

  if (ret) goto cleanup;

  s_voe_initializing = false;
  s_voe_initialized = true;
  InterlockedIncrement(&s_voe_ref);

  return ret;

cleanup:  // do the clean up works when init failed.

  if (_eng_ptr) {
    media_engine_delete(_eng_ptr);
    _eng_ptr = 0;
  }

  s_voe_initialized = false;
  return ret;
}

EXTERNC int voe_release() {
  if (s_voe_initializing) return -1;

  if (!s_voe_initialized) return -1;

  InterlockedDecrement(&s_voe_ref);

  if (s_voe_ref != 0) return 0;  // there are others using engine now.

  s_voe_initializing = true;

  if (_voice_volume) {
    media_engine_release_voice_volume(_voice_volume);
    _voice_volume = 0;
  }

  if (_audio_codec) {
    media_engine_release_voe_codec(_audio_codec);
    _audio_codec = 0;
  }

  if (_eng_hardware) {
    media_engine_release_hardware(_eng_hardware);
    _eng_hardware = 0;
  }
    
  if (_eng_apm) {
    media_engine_release_apm(_eng_apm);
    _eng_apm = 0;
  }

  if (_voice_file) {
    media_engine_release_voe_file(_voice_file);
    _voice_file = 0;
  }
  
  if (_eng_dbg) {
    media_engine_release_debuger(_eng_dbg);
    _eng_dbg = 0;
  }

  if (_eng_base) {
    media_engine_release_voe_base(_eng_base);
    _eng_base = 0;
  }

  if (_eng_network_ptr) {
    media_engine_release_network(_eng_network_ptr);
    _eng_network_ptr = 0;
  }

  if (_eng_ptr) {
    media_engine_delete(_eng_ptr);
    _eng_ptr = 0;
  }

  s_voe_initializing = false;
  s_voe_initialized = false;
  return 0;
}

EXTERNC int voe_get_play_device_count(int &dev_num) {
  if (0 == _eng_hardware) return 0;

  dev_num = 0;
  return engine_hardware_get_num_play_dev(_eng_hardware, dev_num);
}

EXTERNC int voe_get_play_device_name(int idx, char strNameUTF8[128],
                                     char strGuidUTF8[128]) {
  if (_eng_hardware) {
    return engine_hardware_get_play_dev_name(_eng_hardware, idx, strNameUTF8,
           strGuidUTF8);
  }

  return -1;
}

EXTERNC int voe_set_play_device(int idx) {
  if (_eng_hardware) {
    return engine_hardware_set_playout_dev(_eng_hardware, idx);
  }

  return -1;
}

EXTERNC int voe_get_play_device_volume(unsigned int &vol) {
	if (_eng_hardware) {
		return media_engine_get_speaker_volume(_voice_volume, vol);
	}

	return -1;
}
 
EXTERNC int voe_set_play_device_volume(unsigned int vol) {
	//if (_eng_hardware) {
	//	return media_engine_set_speaker_volume(_voice_volume, vol);
	//}
	if (_eng_ptr) {
		return media_engine_set_speaker_vol(_eng_ptr, vol);
	}

	return -1;
}

EXTERNC int voe_channel_hold(int chl, bool hold_flag) {
	if (_eng_base) {
		return media_engine_voe_set_hold_status(_eng_base, chl, hold_flag, kHoldPlayOnly);
	}
	 
	return -1;
}

EXTERNC int voe_channel_hold_status(int chl, bool &hold_flag) {
	if (_eng_base) {
		int mode = kHoldPlayOnly;
		return media_engine_voe_get_hold_status(_eng_base, chl, hold_flag, mode);
	}

	return -1;
}

EXTERNC int voe_get_rec_device_count(int &dev_num) {
  if (_eng_hardware) {
    return engine_hardware_get_num_record_dev(_eng_hardware, dev_num);
  }

  return -1;
}

EXTERNC int voe_get_rec_device_name(int idx, char strNameUTF8[128],
                                    char strGuidUTF8[128]) {
  if (_eng_hardware) {
    return engine_hardware_get_record_dev_name(_eng_hardware, idx, strNameUTF8,
           strGuidUTF8);
  }

  return -1;
}

EXTERNC int voe_set_rec_device(int idx) {
  if (_eng_hardware) {
    return engine_hardware_set_record_dev(_eng_hardware, idx, webrtc::kStereoBoth);
  }

  return -1;
}

EXTERNC int voe_get_rec_device_volume(unsigned int &vol) {
  if (_eng_hardware) {
    return media_engine_get_mic_volume(_voice_volume, vol);
  }

  return -1;
}

EXTERNC int voe_set_rec_device_volume(unsigned int vol) {
  if (_eng_ptr) {
   // return media_engine_set_mic_volume(_voice_volume, vol);
	  return media_engine_set_mic_vol(_eng_ptr, vol);
  }

  return -1;
}


EXTERNC int voe_get_audio_codec_count(int &num) {
  num = 0;
  num = media_engine_voe_codec_num_of_codecs(_audio_codec);

  if (num >= 0)
    return 0;
  else
    return -1;
}

EXTERNC int voe_get_audio_codec_detail(int idx, webrtc::CodecInst &inst) {
  return media_engine_voe_codec_get_codec(_audio_codec, idx, inst);
}

EXTERNC int voe_register_audio_transport(int (*outgo_audio_data)(int, uint8_t,
    uint8_t,
    uint16_t, uint32_t, uint32_t, const char *, int)) {
  _audio_transport.outgo_audio_data = outgo_audio_data;
  return media_engine_network_register_transport(_eng_network_ptr,
         &_audio_transport);
}

EXTERNC int voe_unregister_audio_transport() {
  return media_engine_network_register_transport(_eng_network_ptr, 0);
}

EXTERNC int voe_set_transport_data(int chl, uint8_t bit_mask,
                                   uint8_t pt, uint16_t seq, uint32_t tm, uint32_t ssrc, const char * data,
                                   int len) {
  return media_engine_network_income_rtp_raw_data(_eng_network_ptr, chl,
         bit_mask, pt, seq, tm, ssrc, data, len);
}


EXTERNC int voe_create_channel() {
  return media_engine_voe_create_channel(_eng_base);
}

EXTERNC int voe_delete_channel(int chl) {
  return media_engine_voe_delete_channel(_eng_base, chl);
}

EXTERNC int voe_channel_set_codec(int chl, int pt) {
  if (0 == _audio_codec) return -1;

  CodecInst codec;
  int ret = -1;
  int idx = -1;
  int codec_cnt = 0;
  voe_get_audio_codec_count(codec_cnt);

  for(int i = 0; i < codec_cnt; ++ i) {
    if (voe_get_audio_codec_detail(i, codec)) continue;

    if (codec.pltype == pt) {
      idx = i;
      break;
    }
  }

  ret = voe_get_audio_codec_detail(idx, codec);

  if (0 == ret) {
    if (strncmp(codec.plname, "ISAC", 4) == 0) {
      // Set iSAC to adaptive mode by default.
      codec.rate = -1;
    }

    ret = media_engine_voe_codec_set_send_codec(_audio_codec, chl, codec);
  }

  return ret;
}

EXTERNC int voe_channel_get_codec(int chl, int &pt) {
  if (0 == _audio_codec) return -1;

  CodecInst codec;
  int ret = -1;
  pt = -1;

  ret = media_engine_voe_codec_get_send_codec(_audio_codec, chl, codec);

  if (0 == ret) {
    pt = codec.pltype;
  }

  return ret;
}

EXTERNC int voe_channel_start_send(int chl) {
  return media_engine_voe_start_send(_eng_base, chl);
}

EXTERNC int voe_channel_stop_send(int chl) {
  return media_engine_voe_stop_send(_eng_base, chl);
}

EXTERNC int voe_channel_send_status(int chl, bool &status) {
  return media_engine_voe_send_status(_eng_base, chl, status);
}

EXTERNC int voe_channel_start_receive(int chl) {
  return media_engine_voe_start_receive(_eng_base, chl);
}

EXTERNC int voe_channel_stop_receive(int chl) {
  return media_engine_voe_stop_receive(_eng_base, chl);
}

EXTERNC int voe_channel_start_playout(int chl) {
  return media_engine_voe_start_playout(_eng_base, chl);
}

EXTERNC int voe_channel_stop_playout(int chl) {
  return media_engine_voe_stop_playout(_eng_base, chl);
}

EXTERNC int voe_channel_enable_packet_combine(int chl, bool enable,
    int num_combined) {
  return media_engine_voe_enable_data_combine(
           _eng_base, chl, enable, num_combined);
}

EXTERNC int voe_channel_enable_packet_separate(int chl, bool enable) {
  return media_engine_voe_enable_data_separate(_eng_base, chl, enable);
}

EXTERNC int voe_get_ns_mode(bool &enable, int &mode) {
  int ret = -1;

  if (_eng_apm) {
    return media_engine_apm_get_ns_status(_eng_apm, enable, mode);
  }

  return ret;
}

EXTERNC int voe_set_ns_mode(bool enable, int mode) {
  if (_eng_apm) {
    return media_engine_apm_set_ns_status(_eng_apm, enable, mode);
  }

  return -1;
}

EXTERNC int voe_get_rx_ns_mode(int chl, bool &enable, int &mode) {
  if (_eng_apm) {
    return media_engine_apm_get_rx_ns_status(_eng_apm, chl, enable, mode);
  }

  return -1;
}

EXTERNC int voe_set_rx_ns_mode(int chl, bool enable, int mode) {
  if (_eng_apm) {
    return media_engine_apm_set_rx_ns_status(_eng_apm, chl, enable, mode);
  }

  return -1;
}

EXTERNC int voe_get_ec_mode(bool &enable, int &mode) {
  if (_eng_apm) {
    return media_engine_apm_get_ec_status(_eng_apm, enable, mode);
  }

  return -1;
}

EXTERNC int voe_set_ec_mode(bool enable, int mode) {
  if (_eng_apm) {
    return media_engine_apm_set_ec_status(_eng_apm, enable, mode);
  }

  return -1;
}

EXTERNC int voe_channel_set_vad(int chl, bool enable, int mode,
                                bool dtx_disable) {
  if (_eng_apm)
    return media_engine_apm_set_vad_status(_eng_apm, chl, enable, mode,
                                           dtx_disable);

  return -1;
}

EXTERNC int voe_channel_get_vad(int chl, bool &enable, int &mode,
                                bool &dtx_disable) {
  if (_eng_apm)
    return media_engine_apm_get_vad_status(_eng_apm, chl, enable, mode,
                                           dtx_disable);

  return -1;
}

EXTERNC int voe_get_agc_mode(bool &enable, int &mode) {
  if (_eng_apm) {
    return media_engine_apm_get_agc_status(_eng_apm, enable, mode);
  }

  return -1;
}

EXTERNC int voe_set_agc_mode(bool enable, int mode) {
  if (_eng_apm) {
    return media_engine_apm_set_agc_status(_eng_apm, enable, mode);
  }

  return -1;
}

EXTERNC int voe_set_rx_agc_mode(int chl, bool enable, int mode) {
  if (_eng_apm) {
    return media_engine_apm_set_rx_agc_status(_eng_apm, chl, enable, mode);
  }

  return -1;
}

EXTERNC int voe_get_rx_agc_mode(int chl, bool &enable, int &mode) {
  if (_eng_apm) {
    return media_engine_apm_get_rx_agc_status(_eng_apm, chl, enable, mode);
  }

  return -1;
}

EXTERNC int voe_set_aecm_mode(bool enable, int mode) {
  if (_eng_apm) {
    return media_engine_apm_set_aecm_mode(_eng_apm, mode, enable);
  }

  return -1;
}

EXTERNC int voe_get_aecm_mode(bool &enable, int &mode) {
  if (_eng_apm) {
    return media_engine_apm_get_aecm_mode(_eng_apm, mode, enable);
  }

  return -1;
}

EXTERNC int voe_start_record_mircophone(const char* file_name_utf8,
                                        int compress_type, int max_size) {
  if (_voice_file) {
    return media_engine_voice_start_record_mircophone(_voice_file, file_name_utf8,
           compress_type, max_size);
  }

  return -1;
}

EXTERNC int voe_stop_record_mircophone() {
  if (_voice_file) {
    return media_engine_voice_stop_record_mircophone(_voice_file);
  }

  return -1;
}

EXTERNC int voe_channel_start_play_file_as_mic(int channel,
    const char* mic_file_utf8, bool loop, bool mix_with_mic, int format,
    float volume_scale /* = 1.0f */) {
  if (_voice_file)
    return media_engine_voice_start_play_file_as_mic(_voice_file, channel,
           mic_file_utf8, loop, mix_with_mic, format, volume_scale);

  return 0;
}

EXTERNC int voe_channel_stop_play_file_as_mic(int chl) {
  if (_voice_file)
    return media_engine_voice_stop_play_file_as_mic(_voice_file, chl);

  return -1;
}

EXTERNC int voe_channel_start_play_file_locally(int channel,
  const char *file_name_utf8, bool loop, int format, float volume_scale) {
    if (_voice_file) 
      return media_engine_voice_start_play_file_locally(_voice_file, channel,
      file_name_utf8, loop, format, volume_scale);
    return -1;
}

EXTERNC int voe_channel_stop_play_file_locally(int chl) {
  if (_voice_file)
    return media_engine_voice_stop_play_file_locally(_voice_file, chl);
  return -1;
}

EXTERNC int voe_debug_start_data_dump(int channel, const char* utf8_name, int direction) {
  if (_eng_dbg) {
    return media_engine_debug_strat_rtp_dump(_eng_dbg, channel, utf8_name, direction);
  }
  return -1;
}

EXTERNC int voe_debug_stop_data_dump(int channel, int direction) {
  if (_eng_dbg) {
    return media_engine_debug_stop_rtp_dump(_eng_dbg, channel,direction);
  }
  return -1;
}

EXTERNC int voe_debug_data_dump_actived(int channel, int direction, bool &enable) {
  if (_eng_dbg) {
    int ret = 0;
    ret = media_engine_debug_rtp_dump_active(_eng_dbg, channel, direction);
    if (ret < 0) return ret;
    enable = ret > 0;
    return 0;
  }
  return -1;  
}

EXTERNC int voe_debug_write_dump_date_externinfo(const char* utf8_file_name,
  const char* buf, int buf_len) {
  if (_eng_dbg)
    return media_engine_debug_write_rtp_dump_extern_info(_eng_dbg, utf8_file_name,
    buf, buf_len);
  return -1;
  return 0;
}

EXTERNC int voe_get_speech_input_level()
{
	if(_voice_volume != NULL)
	{
		unsigned int level = 0;
		media_engine_get_speech_input_level(_voice_volume, level);
		level = level * 1000 / 256;

		return level;
	}
	else
	{
		return 0;
	}
}

EXTERNC int voe_get_speech_output_level(int chl)
{
	if(_voice_volume != NULL)
	{
		unsigned int level = 0;
		media_engine_get_speech_output_level(_voice_volume, chl, level);
		level = level * 1000 / 256;

		return level;
	}
	else
	{
		return 0;
	}
}


