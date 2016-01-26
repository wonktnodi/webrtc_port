#include "config.h"
#include "engine_base.h"
#include <assert.h>
#include <malloc.h>
#include "trace.h"

#include "engine_base_impl.h"
#include "voe_hardware.h"
#include "voe_base.h"
#include "voe_codec.h"
#include "engine_network_impl.h"
#include "voe_file.h"
#include "voe_apm.h"
#include "voe_volume.h"
#include "engine_debug_impl.h"
#include "output_mixer.h"
#include "transmit_mixer.h"

using namespace webrtc;

struct media_engine {
  MediaEngineImpl *eng_ptr;
};

struct media_engine_hardware {
  VoEHardwareImpl *voe_hw;
  media_engine *eng_ref;
};

struct media_engine_voice_base {
  VoEBaseImpl *voe_base;
  media_engine *eng_ref;
};

struct media_engine_voice_codec {
  VoECodecImpl *voe_codec;
  media_engine *eng_ref;
};

struct media_engine_network {
  MediaEngineNetworkImpl *network;
  media_engine *eng_ref;
};

struct media_engine_voice_file {
  VoeFileImpl *file;
  media_engine *eng_ref;
};

struct media_engine_apm {
  VoEAudioProcessingImpl *proc;
  media_engine *eng_ref;
};

struct media_engine_voice_volume {
  VoEAudioVolumeImpl *ctrl;
  media_engine *eng_ref;
};

struct media_engine_debuger {
  MediaEngineDebugImpl *dbg;
  media_engine *eng_ref;
};

// Counter to be ensure that we can add a correct ID in all static trace
// methods. It is not the nicest solution, especially not since we already
// have a counter in VoEBaseImpl. In other words, there is room for
// improvement here.
static int32_t gVoiceEngineInstanceCounter = 0;

FUNC_DEL int media_engine_set_trace_filter(const unsigned int filter,
    const unsigned int module) {
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice,
               VoEId(gVoiceEngineInstanceCounter, -1),
               "SetTraceFilter(filter=0x%x)", filter);

  // Remember old filter
  WebRtc_UWord32 oldFilter = 0, oldModule = 0;
  Trace::LevelFilter(oldFilter, oldModule);

  // Set new filter
  WebRtc_Word32 ret = Trace::SetLevelFilter(filter, module);

  // If previous log was ignored, log again after changing filter
  if (kTraceNone == oldFilter || kTraceUndefined == oldModule) {
    WEBRTC_TRACE(kTraceApiCall, kTraceVoice, -1,
                 "SetTraceFilter(filter=0x%x, module=0x%x)", filter, module);
  }

  return (ret);
}

FUNC_DEL int media_engine_set_trace_file(const char* fileNameUTF8,
    const bool addFileCounter /* = false */) {
  int ret = Trace::SetTraceFile(fileNameUTF8, addFileCounter);
  WEBRTC_TRACE(kTraceApiCall, kTraceVoice,
               VoEId(gVoiceEngineInstanceCounter, -1),
               "SetTraceFile(fileNameUTF8=%s, addFileCounter=%d)",
               fileNameUTF8, addFileCounter);
  return (ret);
}

static media_engine* alloc_media_engine() {
  media_engine *obj = new(media_engine);

  if (0 == obj) return obj;

  // allocate object's members.
  obj->eng_ptr = new MediaEngineImpl;
  assert(obj->eng_ptr);
  return obj;
}

FUNC_DEL int media_engine_create(media_engine **eng) {
  media_engine *self = 0;

  self = alloc_media_engine();

  if (self != 0) {
    media_engine_addref(self);
  } else {
    return -1;
  }

  *eng = self;/**/
  return 0;
}

FUNC_DEL int media_engine_delete(media_engine*& eng) {
  if (eng == NULL)
    return -1;

  // Release the reference that was added in GetVoiceEngine.
  int ref = media_engine_release(eng);
  eng = NULL;

  if (ref != 0) {
    WEBRTC_TRACE(kTraceWarning, kTraceVoice, -1,
                 "VoiceEngine::Delete did not release the very last reference.  "
                 "%d references remain.", ref);
  }

  return 0;
}

int media_engine_addref(media_engine *eng) {
  if (!eng || !eng->eng_ptr) return -1;

  return eng->eng_ptr->AddRef();
}

FUNC_DEL int media_engine_release(media_engine *eng) {
  if (!eng || !eng->eng_ptr) return -1;

  int new_ref = eng->eng_ptr->Release();
  assert(new_ref >= 0);

  if (new_ref == 0) {
    WEBRTC_TRACE(kTraceApiCall, kTraceVoice, -1,
                 "media_engine self deleting (MediaEngine=0x%p)", eng);

    delete eng;
  }

  return new_ref;
}

FUNC_DEL int media_engine_init(media_engine* eng) {
  assert(eng);
  return eng->eng_ptr->Init();
}

FUNC_DEL int media_engine_terminate(media_engine* eng) {
  assert(eng);
  return eng->eng_ptr->Terminate();
}

FUNC_DEL int media_engine_set_mic_mute(media_engine* eng, bool enable) {
	TransmitMixer* mixer = eng->eng_ptr->transmit_mixer();
	if(mixer != NULL){
		mixer->SetMute(enable);
	}
	else {
		WEBRTC_TRACE(kTraceError, kTraceVoice, -1,
			"TransmitMixer is NULL (MediaEngine=0x%p)", eng);
	}

	return 0;
}

FUNC_DEL int media_engine_set_mic_vol(media_engine* eng, int vol) {
	TransmitMixer* mixer = eng->eng_ptr->transmit_mixer();
	if(mixer != NULL){
		float scale = vol * 1.0 / 255;
		mixer->SetScale(scale);
	}
	else {
		WEBRTC_TRACE(kTraceError, kTraceVoice, -1, "TransmitMixer is NULL (MediaEngine=0x%p)", eng);
	}

	return 0;
}

FUNC_DEL int media_engine_set_speaker_mute(media_engine* eng, bool enable){
	OutputMixer* mixer = eng->eng_ptr->output_mixer();
	if(mixer != NULL){
		mixer->SetMute(enable);
	}
	else {
		WEBRTC_TRACE(kTraceError, kTraceVoice, -1, "OutputMixer is NULL (MediaEngine=0x%p)", eng);
	}

	return 0;
}

FUNC_DEL int media_engine_set_speaker_vol(media_engine* eng, int vol) {
	OutputMixer* mixer = eng->eng_ptr->output_mixer();
	if(mixer != NULL){
		float scale = vol * 1.0 / 255;
		mixer->SetScale(scale);
	}
	else {
		WEBRTC_TRACE(kTraceError, kTraceVoice, -1, "OutputMixer is NULL (MediaEngine=0x%p)", eng);
	}

	return 0;
}

FUNC_DEL int media_engine_get_hardware(media_engine* eng,
                                       media_engine_hardware*& mgr) {
  media_engine_hardware *obj = new media_engine_hardware;

  if (0 == obj) return -1;

  obj->voe_hw = new VoEHardwareImpl(eng->eng_ptr);

  if (0 == obj->voe_hw) return -1;

  obj->eng_ref = eng;
  media_engine_addref(eng);
  mgr = obj;
  return 0;
}

FUNC_DEL int media_engine_release_hardware(media_engine_hardware*& hdr) {
  assert(hdr && hdr->voe_hw);
  media_engine *eng = hdr->eng_ref;
  delete hdr->voe_hw;
  delete hdr;

  if (eng)
    media_engine_release(eng);

  hdr = 0;
  return 0;

}

FUNC_DEL int engine_hardware_get_num_record_dev(media_engine_hardware *mgr,
    int& cnt) {
  assert(mgr && mgr->voe_hw);
  return mgr->voe_hw->GetNumOfRecordingDevices(cnt);
}

FUNC_DEL int engine_hardware_get_record_dev_name(media_engine_hardware* mgr,
    int index, char strNameUTF8[128], char strGuidUTF8[128]) {
  assert(mgr && mgr->voe_hw);
  return mgr->voe_hw->GetRecordingDeviceName(index, strNameUTF8, strGuidUTF8);
}

FUNC_DEL int engine_hardware_get_num_play_dev(media_engine_hardware *mgr,
    int& cnt) {
  assert(mgr && mgr->voe_hw);
  return mgr->voe_hw->GetNumOfPlayoutDevices(cnt);
}

FUNC_DEL int engine_hardware_get_play_dev_name(media_engine_hardware* mgr,
    int index, char strNameUTF8[128], char strGuidUTF8[128]) {
  assert(mgr && mgr->voe_hw);
  return mgr->voe_hw->GetPlayoutDeviceName(index, strNameUTF8, strGuidUTF8);
}

FUNC_DEL
int engine_hardware_set_record_dev(media_engine_hardware *mgr, int index,
                                   int record_channel) {
  assert(mgr && mgr->voe_hw);
  return mgr->voe_hw->SetRecordingDevice(index, (StereoChannel)record_channel);
}

FUNC_DEL
int engine_hardware_set_playout_dev(media_engine_hardware *mgr, int index) {
  assert(mgr && mgr->voe_hw);
  return mgr->voe_hw->SetPlayoutDevice(index);
}

FUNC_DEL
int engine_hardware_get_cur_record_dev(media_engine_hardware *mgr, int &idx) {
  assert(mgr && mgr->voe_hw);
  return mgr->voe_hw->InputDevice(idx);
}

FUNC_DEL
int engine_hardware_get_cur_playout_dev(media_engine_hardware *mgr, int &idx) {
  assert(mgr && mgr->voe_hw);
  return mgr->voe_hw->OutputDevice(idx);
}


// voice base APIs.

FUNC_DEL int media_engine_get_voe_base(media_engine* eng,
                                       media_engine_voice_base *& base) {
  media_engine_voice_base *obj = new media_engine_voice_base;

  if (0 == obj) return -1;

  obj->voe_base = new VoEBaseImpl(eng->eng_ptr);

  if (0 == obj->voe_base) return -1;

  obj->eng_ref = eng;
  media_engine_addref(eng);
  base = obj;
  return 0;
}

FUNC_DEL int media_engine_release_voe_base(media_engine_voice_base*& base) {
  assert(base);
  media_engine *eng = base->eng_ref;
  delete base->voe_base;
  delete base;

  if (eng)
    media_engine_release(eng);

  base = 0;
  return 0;
}

FUNC_DEL int media_engine_voe_create_channel(media_engine_voice_base *base) {
  assert(base);
  int chl = base->voe_base->CreateChannel();

  return chl;
}

FUNC_DEL int media_engine_voe_delete_channel(media_engine_voice_base *base,
    int channel) {
  assert(base);
  return base->voe_base->DeleteChannel(channel);
}

FUNC_DEL int
media_engine_voe_start_send(media_engine_voice_base *base, int chl) {
  assert(base);
  return base->voe_base->StartSend(chl);
}

FUNC_DEL int
media_engine_voe_stop_send(media_engine_voice_base *base, int chl) {
  assert(base);
  return base->voe_base->StopSend(chl);
}

FUNC_DEL int
media_engine_voe_send_status(media_engine_voice_base *base, int chl,
                             bool &sending) {
  assert(base);
  return base->voe_base->SendingStatus(chl, sending);
}

FUNC_DEL int media_engine_voe_start_receive(media_engine_voice_base *base,
    int chl) {
  assert(base);
  return base->voe_base->StartReceive(chl);
}

FUNC_DEL int media_engine_voe_stop_receive(media_engine_voice_base *base,
    int chl) {
  assert(base);
  return base->voe_base->StopReceive(chl);
}

FUNC_DEL int media_engine_voe_start_playout(media_engine_voice_base *base,
    int chl) {
  assert(base);
  return base->voe_base->StartPlayout(chl);
}

FUNC_DEL int media_engine_voe_stop_playout(media_engine_voice_base *base,
    int chl) {
  assert(base);
  return base->voe_base->StopPlayout(chl);
}

FUNC_DEL int media_engine_voe_get_delay_with_playouttimestamp(
  media_engine_voice_base *base, int chl, uint32_t timestamp, uint32_t &delay) {
  assert(base);
  return base->voe_base->GetDelayWithPlayoutTimeStamp(chl, timestamp, delay);
}

FUNC_DEL int media_engine_voe_get_network_statistics(
  media_engine_voice_base *base, int channel, NetworkStatistics &stat) {
  assert(base);
  return base->voe_base->GetNetworkStatistics(channel, stat);
}

FUNC_DEL int media_engine_voe_set_hold_status(
  media_engine_voice_base *base, int channel, bool enable, int mode) {
  assert(base);
  return base->voe_base->SetOnHoldStatus(channel, enable, (OnHoldModes)mode);
}

FUNC_DEL int media_engine_voe_get_hold_status(
  media_engine_voice_base *base, int channel, bool &enable, int &mode) {
  assert(base);
  OnHoldModes ret_mode = (OnHoldModes) - 1;
  int ret;
  ret = base->voe_base->GetOnHoldStatus(channel, enable, ret_mode);

  if (!ret) mode = ret_mode;

  return ret;
}

FUNC_DEL int media_engine_voe_get_last_error(media_engine_voice_base *base) {
  assert(base);
  return base->voe_base->LastError();
}

FUNC_DEL int media_engine_voe_enable_data_combine(media_engine_voice_base *base,
    int chl, bool enable, uint32_t max_packet) {
  assert(base);
  return base->voe_base->EnablePacketCombine(chl, enable, max_packet);
}

FUNC_DEL int media_engine_voe_enable_data_separate(media_engine_voice_base
    *base,
    int chl, bool enable) {
  assert(base);
  return base->voe_base->EnablePacketSeparate(chl, enable);
}

// voice codec APIs
FUNC_DEL int media_engine_get_voe_codec(media_engine* eng,
                                        media_engine_voice_codec *&codec) {
  media_engine_voice_codec *obj = new media_engine_voice_codec;

  if (0 == obj) return -1;

  obj->voe_codec = new VoECodecImpl(eng->eng_ptr);

  if (0 == obj->voe_codec) return -1;

  obj->eng_ref = eng;
  media_engine_addref(eng);
  codec = obj;
  return 0;
}

FUNC_DEL int media_engine_release_voe_codec(media_engine_voice_codec *&codec) {
  assert(codec);
  media_engine *eng = codec->eng_ref;
  delete codec->voe_codec;
  delete codec;

  if (eng)
    media_engine_release(eng);

  codec = 0;
  return 0;
}

FUNC_DEL int media_engine_voe_codec_num_of_codecs(
  media_engine_voice_codec* codec) {
  assert(codec);
  return codec->voe_codec->NumOfCodecs();
}

FUNC_DEL int media_engine_voe_codec_get_codec(
  media_engine_voice_codec* inst, int index, CodecInst& codec) {
  assert(inst);
  return inst->voe_codec->GetCodec(index, codec);
}

FUNC_DEL int media_engine_voe_codec_get_codec_by_payload_type(
  media_engine_voice_codec *inst, uint8_t payload_type, CodecInst& codec) {
  return inst->voe_codec->GetCodecByPayloadType(payload_type, codec);
}

FUNC_DEL int media_engine_voe_codec_set_send_codec(
  media_engine_voice_codec* inst, int channel, const CodecInst& codec) {
  assert(inst);
  return inst->voe_codec->SetSendCodec(channel, codec);
}

FUNC_DEL int media_engine_voe_codec_get_send_codec(
  media_engine_voice_codec* inst, int channel, CodecInst& codec) {
  assert(inst);
  return inst->voe_codec->GetSendCodec(channel, codec);
}

FUNC_DEL int media_engine_voe_codec_get_rec_codec(
  media_engine_voice_codec* inst, int channel, CodecInst& codec) {
  assert(inst);
  return inst->voe_codec->GetRecCodec(channel, codec);
}

// Engine network APIs.
FUNC_DEL int media_engine_get_network(media_engine* eng,
                                      media_engine_network*& net) {
  media_engine_network *obj = new media_engine_network;

  if (0 == obj) return -1;

  obj->network = new MediaEngineNetworkImpl(eng->eng_ptr);

  if (0 == obj->network) return -1;

  obj->eng_ref = eng;
  media_engine_addref(eng);
  net = obj;

  return 0;
}

FUNC_DEL int media_engine_release_network(media_engine_network*& net) {
  assert(net);
  media_engine *eng = net->eng_ref;
  delete net->network;
  delete net;

  if (eng)
    media_engine_release(eng);

  net = 0;
  return 0;
}

FUNC_DEL int media_engine_network_register_transport(
  media_engine_network* net, engine_transport* transport) {
  assert(net);
  return net->network->RegisterTransport(transport);
}

FUNC_DEL int media_engine_network_income_rtp_data(
  media_engine_network* net, int channel, const char* data, int len) {
  assert(net);
  return net->network->ReceivedRTPPacket(channel, data, len);
}

FUNC_DEL int
media_engine_network_income_rtp_raw_data(media_engine_network* net,
    int channel, uint8_t bit_mask, uint8_t pt, uint16_t seq, uint32_t tm,
    uint32_t ssrc,
    const char * data, int len) {
  assert(net);
  return net->network->ReceivedRawPacket(channel, bit_mask, pt, seq, tm,
                                         ssrc, data, len);
}

FUNC_DEL int media_engine_network_income_rtcp_data(
  media_engine_network* net, int channel, const char* data, int len) {
  assert(net);
  return net->network->ReceivedRTCPPacket(channel, data, len);
}

// Engine voice file APIs
FUNC_DEL int media_engine_get_voe_file(
  media_engine* eng, media_engine_voice_file*& file) {
  media_engine_voice_file *obj = new media_engine_voice_file;

  if (0 == obj) return -1;

  obj->file = new VoeFileImpl(eng->eng_ptr);

  if (0 == obj->file) return -1;

  obj->eng_ref = eng;
  media_engine_addref(eng);
  file = obj;

  return 0;
}

FUNC_DEL int media_engine_release_voe_file(media_engine_voice_file*& file) {
  assert(file);
  media_engine *eng = file->eng_ref;
  delete file->file;
  delete file;

  if (eng)
    media_engine_release(eng);

  file = 0;
  return 0;
}

FUNC_DEL
int media_engine_voice_start_play_file_as_mic(media_engine_voice_file* file,
    int channel, const char* mic_file_utf8, bool loop, bool mix_with_mic,
    int format,
    float volume_scale /* = 1.0f */ ) {
  assert(file);

  return file->file->StartPlayingFileAsMicrophone(channel, mic_file_utf8,
         loop, mix_with_mic, (FileFormats)format, volume_scale);
}

FUNC_DEL
int media_engine_voice_stop_play_file_as_mic(media_engine_voice_file* file,
    int channel) {
  assert(file);
  return file->file->StopPlayingFileAsMicrophone(channel);;
}

FUNC_DEL
int media_engine_voice_set_mic_file_scale(media_engine_voice_file* file,
    int channel, int scale) {
  assert(file);
  return 0;
}

FUNC_DEL
int media_engine_voice_is_play_file_as_mic(media_engine_voice_file* file,
    int channel) {
  assert(file);
  return file->file->IsPlayingFileAsMicrophone(channel);
}

FUNC_DEL
int media_engine_voice_start_play_file_locally(media_engine_voice_file* file,
    int channel, const char *file_name_utf8, bool loop, int format,
    float volume_scale /* = 1.0f */) {
  assert(file);
  return file->file->StartPlayingFileLocally(channel, file_name_utf8, loop,
         (FileFormats)format, volume_scale);
  return 0;
}

FUNC_DEL
int media_engine_voice_stop_play_file_locally(media_engine_voice_file* file,
    int channel) {
  assert(file);
  return file->file->StopPlayingFileLocally(channel);
}

FUNC_DEL
int media_engine_voice_is_play_file_locally(media_engine_voice_file* file,
    int channel) {
  assert(file);
  return file->file->IsPlayingFileLocally(channel);
}


FUNC_DEL
int media_engine_voice_start_record_playout(media_engine_voice_file* file,
    int channel, const char* file_name_utf8) {
  assert(file);
  return 0;
}

FUNC_DEL
int media_engine_voice_stop_record_playout(media_engine_voice_file* file,
    int channel) {
  assert(file);
  return 0;
}

FUNC_DEL
int media_engine_voice_start_record_mircophone(media_engine_voice_file* file,
    const char* file_name_utf8, int compress_type, int max_size) {
  assert(file);
  return file->file->StartRecordingMicrophone(file_name_utf8, 0, max_size);
}

FUNC_DEL
int media_engine_voice_stop_record_mircophone(media_engine_voice_file* file) {
  assert(file);
  return file->file->StopRecordingMicrophone();
}

// engine APM APIs
FUNC_DEL int media_engine_get_apm(
  media_engine* eng, media_engine_apm*& apm) {
  media_engine_apm *obj = new media_engine_apm;

  if (0 == obj) return -1;

  obj->proc = new VoEAudioProcessingImpl(eng->eng_ptr);

  if (0 == obj->proc) return -1;

  obj->eng_ref = eng;
  media_engine_addref(eng);
  apm = obj;

  return 0;
}

FUNC_DEL int media_engine_release_apm(media_engine_apm*& apm) {
  assert(apm);
  media_engine *eng = apm->eng_ref;
  delete apm->proc;
  delete apm;

  if (eng)
    media_engine_release(eng);

  apm = 0;
  return 0;
}

FUNC_DEL int media_engine_apm_set_ns_status(media_engine_apm *apm,
    bool enable, int mode) {
  assert(apm);
  return apm->proc->SetNsStatus(enable, (NsModes)mode);
}

FUNC_DEL int media_engine_apm_get_ns_status(media_engine_apm *apm,
    bool& enable, int& mode) {
  assert(apm);
  NsModes ret_mode;
  int ret = apm->proc->GetNsStatus(enable, ret_mode);
  mode = ret_mode;
  return ret;
}

FUNC_DEL int media_engine_apm_set_rx_ns_status(media_engine_apm *apm,
    int channel, bool enable, int mode) {
  assert(apm);
  return apm->proc->SetRxNsStatus(channel, enable, (NsModes)mode);
}

FUNC_DEL int media_engine_apm_get_rx_ns_status(media_engine_apm *apm,
    int channel, bool& enable, int& mode) {
  assert(apm);
  NsModes ret_mode;
  int ret = apm->proc->GetRxNsStatus(channel, enable, ret_mode);
  mode = ret_mode;
  return ret;
}

FUNC_DEL int media_engine_apm_set_agc_status(media_engine_apm *apm,
    bool enable, int mode) {
  assert(apm);
  return apm->proc->SetAgcStatus(enable, (AgcModes)mode);
}

FUNC_DEL int media_engine_apm_get_agc_status(media_engine_apm *apm,
    bool& enable, int& mode) {
  assert(apm);
  AgcModes ret_mode;
  int ret = apm->proc->GetAgcStatus(enable, ret_mode);
  mode = ret_mode;
  return ret;
}

FUNC_DEL int media_engine_apm_set_rx_agc_status(media_engine_apm *apm,
    int channel, bool enable, int mode) {
  assert(apm);
  return apm->proc->SetRxAgcStatus(channel, enable, (AgcModes)mode);
}

FUNC_DEL int media_engine_apm_get_rx_agc_status(media_engine_apm *apm,
    int channel, bool& enable, int& mode) {
  assert(apm);
  AgcModes ret_mode;
  int ret = apm->proc->GetRxAgcStatus(channel, enable, ret_mode);
  mode = ret_mode;
  return ret;
}

FUNC_DEL int media_engine_apm_set_ec_status(media_engine_apm *apm,
    bool enable, int mode) {
  assert(apm);
  return apm->proc->SetEcStatus(enable, (EcModes)mode);
}

FUNC_DEL int media_engine_apm_get_ec_status(media_engine_apm *apm,
    bool& enable, int& mode) {
  assert(apm);
  EcModes ret_mode;
  int ret = apm->proc->GetEcStatus(enable, ret_mode);
  mode = ret_mode;
  return ret;
}

FUNC_DEL int media_engine_apm_set_aecm_mode(media_engine_apm *apm,
    int mode, bool enable_cng) {
  assert(apm);
  return apm->proc->SetAecmMode((AecmModes)mode, enable_cng);
}

FUNC_DEL int media_engine_apm_get_aecm_mode(media_engine_apm *apm,
    int& mode, bool &enable_cng) {
  assert(apm);
  AecmModes ret_mode;
  int ret = apm->proc->GetAecmMode(ret_mode, enable_cng);
  mode = ret_mode;
  return ret;
}

FUNC_DEL int media_engine_apm_enable_hpf(media_engine_apm *apm,
    bool enable) {
  assert(apm);
  return apm->proc->EnableHighPassFilter(enable);
}

FUNC_DEL int media_engine_apm_is_hpf_enabled(media_engine_apm *apm,
    bool& enable) {
  assert(apm);
  enable = apm->proc->IsHighPassFilterEnabled();
  return 0;
}

FUNC_DEL int media_engine_apm_set_vad_status(media_engine_apm *apm,
    int channel, bool enable, int mode, bool disableDTX) {
  assert(apm);
  return apm->proc->SetVADStatus(channel, enable, (VadModes)mode, disableDTX);
}

FUNC_DEL int media_engine_apm_get_vad_status(media_engine_apm *apm,
    int channel, bool &enable, int &mode, bool &disableDTX) {
  assert(apm);
  VadModes ret_mode;
  int ret = 0;
  ret = apm->proc->GetVADStatus(channel, enable, ret_mode, disableDTX);
  mode = ret_mode;
  return ret;
}

// voice volume control APIs
FUNC_DEL int media_engine_get_voice_volume(
  media_engine* eng, media_engine_voice_volume*& vol) {
  media_engine_voice_volume *obj = new media_engine_voice_volume;

  if (0 == obj) return -1;

  obj->ctrl = new VoEAudioVolumeImpl(eng->eng_ptr);

  if (0 == obj->ctrl) return -1;

  obj->eng_ref = eng;
  media_engine_addref(eng);
  vol = obj;

  return 0;
}

FUNC_DEL
int media_engine_release_voice_volume(media_engine_voice_volume*& vol) {
  assert(vol);
  media_engine *eng = vol->eng_ref;
  delete vol->ctrl;
  delete vol;

  if (eng)
    media_engine_release(eng);

  vol = 0;
  return 0;
}

FUNC_DEL
int media_engine_get_speaker_volume(media_engine_voice_volume *obj,
                                    unsigned int&volume) {
  assert(obj);
  return obj->ctrl->GetSpeakerVolume(volume);
}

FUNC_DEL
int media_engine_set_speaker_volume(media_engine_voice_volume *obj,
                                    unsigned int volume) {
  assert(obj);
  return obj->ctrl->SetSpeakerVolume(volume);
}

FUNC_DEL
int media_engine_set_system_output_mute(media_engine_voice_volume *obj,
                                        bool enable) {
  assert(obj);
  return obj->ctrl->SetSystemOutputMute(enable);
}

FUNC_DEL int media_engine_get_system_output_mute(media_engine_voice_volume *obj,
    bool &enable) {
  assert(obj);
  return obj->ctrl->GetSystemOutputMute(enable);
}

FUNC_DEL
int media_engine_set_mic_volume(media_engine_voice_volume *obj,
                                unsigned int volume) {
  assert(obj);
  return obj->ctrl->SetMicVolume(volume);
}

FUNC_DEL
int media_engine_get_mic_volume(media_engine_voice_volume *obj,
                                unsigned int&volume) {
  assert(obj);
  return obj->ctrl->GetMicVolume(volume);
}

FUNC_DEL
int media_engine_set_input_mute(media_engine_voice_volume *obj,
                                int channel, bool enable) {
  assert(obj);
  return obj->ctrl->SetInputMute(channel, enable);
}

FUNC_DEL
int media_engine_get_input_mute(media_engine_voice_volume *obj,
                                int channel, bool &enable) {
  assert(obj);
  return obj->ctrl->GetInputMute(channel, enable);
}

FUNC_DEL
int media_engine_set_system_input_mute(media_engine_voice_volume *obj,
                                       bool enable) {
  assert(obj);
  return obj->ctrl->SetSystemInputMute(enable);
}

FUNC_DEL
int media_engine_get_system_input_mute(media_engine_voice_volume *obj,
                                       bool &enable) {
  assert(obj);
  return obj->ctrl->GetSystemInputMute(enable);
}

FUNC_DEL
int media_engine_get_speech_input_level(media_engine_voice_volume *obj,
                                        unsigned int &level) {
  assert(obj);
  return obj->ctrl->GetSpeechInputLevel(level);
}

FUNC_DEL
int media_engine_get_speech_output_level(media_engine_voice_volume *obj,
    int channel, unsigned int &level) {
  assert(obj);
  return obj->ctrl->GetSpeechOutputLevel(channel, level);
}

FUNC_DEL
int media_engine_get_speech_input_level_range(
  media_engine_voice_volume *obj, unsigned int &level) {
  assert(obj);
  return obj->ctrl->GetSpeechInputLevelFullRange(level);
}

FUNC_DEL
int media_engine_get_speech_output_level_range(
  media_engine_voice_volume *obj, int channel, unsigned int &level) {
  assert(obj);
  return obj->ctrl->GetSpeechOutputLevel(channel, level);
}

FUNC_DEL
int media_engine_set_channel_output_volume_scaling(
  media_engine_voice_volume *obj, int channel, float scaling) {
  assert(obj);
  return obj->ctrl->SetChannelOutputVolumeScaling(channel, scaling);
}

FUNC_DEL
int media_engine_get_channel_output_volume_scaling(
  media_engine_voice_volume *obj, int channel, float &scaling) {
  assert(obj);
  return obj->ctrl->GetChannelOutputVolumeScaling(channel, scaling);
}

FUNC_DEL
int meida_engine_set_output_volume_pan(
  media_engine_voice_volume *obj, int channel, float left, float right) {
  assert(obj);
  return obj->ctrl->SetOutputVolumePan(channel, left, right);
}

FUNC_DEL
int meida_engine_get_output_volume_pan(
  media_engine_voice_volume *obj, int channel, float &left, float &right) {
  assert(obj);
  return obj->ctrl->GetOutputVolumePan(channel, left, right);
}


// media engine debug APIs
FUNC_DEL
int media_engine_get_debuger(media_engine* eng, media_engine_debuger *&dbg) {
  media_engine_debuger *obj = new media_engine_debuger;

  if (0 == obj) return -1;

  obj->dbg = new MediaEngineDebugImpl(eng->eng_ptr);

  if (0 == obj->dbg) return -1;

  obj->eng_ref = eng;
  media_engine_addref(eng);
  dbg = obj;

  return 0;
}

FUNC_DEL
int media_engine_release_debuger(media_engine_debuger *&dbg) {
  assert(dbg);
  media_engine *eng = dbg->eng_ref;
  delete dbg->dbg;
  delete dbg;

  if (eng)
    media_engine_release(eng);

  dbg = 0;
  return 0;
}

FUNC_DEL
int media_engine_debug_strat_rtp_dump(
  media_engine_debuger *obj, int channel, const char* utf8_name, int direction) {
  assert(obj);
  return obj->dbg->StartRTPDump(channel, utf8_name,
                                (webrtc::RTPDirections)direction);
}

FUNC_DEL
int media_engine_debug_stop_rtp_dump(
  media_engine_debuger *obj, int channel, int direction) {
  assert(obj);
  return obj->dbg->StopRTPDump(channel, (webrtc::RTPDirections)direction);
}

FUNC_DEL
int media_engine_debug_rtp_dump_active(
  media_engine_debuger *obj, int channel, int direction) {
  assert(obj);
  return obj->dbg->RTPDumpIsActive(channel, (webrtc::RTPDirections)direction);
}

FUNC_DEL
int media_engine_debug_write_rtp_dump_extern_info(
  media_engine_debuger *obj, const char* utf8_file_name,
  const char* buf, int buf_len) {
  assert(obj);

  FILE* tmp_id = NULL;
  wchar_t wide_file_name[1024];
  wide_file_name[0] = 0;

  MultiByteToWideChar(CP_UTF8,
                      0,  // UTF8 flag
                      utf8_file_name,
                      -1,  // Null terminated string
                      wide_file_name,
                      1024);

  tmp_id = _wfopen(wide_file_name, L"rb+");

  if (0 == tmp_id) {
    return -1;
  }

  int ch = getc(tmp_id);
  int pos = 0;

  while (ch != '\n') {
    ch = getc(tmp_id);
    ++ pos;
  }

  fseek(tmp_id, pos + 1, SEEK_SET);
  fwrite(buf, 1, buf_len, tmp_id);
  fflush(tmp_id);
  fclose(tmp_id);

  return 0;
}