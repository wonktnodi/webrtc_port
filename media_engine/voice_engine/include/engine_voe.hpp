#ifndef __MEDIA_ENGINE_VOE_HPP__
#define __MEDIA_ENGINE_VOE_HPP__

#include "engine_base.hpp"
#include "engine_voe.h"

class MediaEngineVoe {
public:
  static MediaEngineVoe* GetInterface(MediaEngine *eng) {
    MediaEngineVoe *obj = 0;
    obj = new MediaEngineVoe();
    if (0 == obj) return 0;

    int ret = media_engine_get_voe_base(eng->_eng_ptr, obj->_obj_ptr);
    if (ret) {
      delete obj;
      return 0;
    }

    return obj;
  }

  static int Release(MediaEngineVoe *hdr) {
    assert(hdr);
    media_engine_release_voe_base(hdr->_obj_ptr);
    delete hdr;
    return 0;
  }

public:
  int CreateChannel() {
    return media_engine_voe_create_channel(_obj_ptr);
  }

  int DeleteChannel(int chl) {
    return media_engine_voe_delete_channel(_obj_ptr, chl);
  }

  int StartSend(int chl) {
    return media_engine_voe_start_send(_obj_ptr, chl);
  }
  
  int StopSend(int chl) {
    return media_engine_voe_stop_send(_obj_ptr, chl);
  }

  // Get sending status from a specified |channel|.
  int SendingStatus(int channel, bool &sending) {
    return media_engine_voe_send_status(_obj_ptr, channel, sending);
  }

  int StartReceiving(int chl) {
    return media_engine_voe_start_receive(_obj_ptr, chl);
  }

  int StopReceiving(int chl) {
    return media_engine_voe_stop_receive(_obj_ptr, chl);
  }

  int StartPlayout(int chl) {
    return media_engine_voe_start_playout(_obj_ptr, chl);
  }

  int StopPlayout(int chl) {
    return media_engine_voe_stop_playout(_obj_ptr, chl);
  }

  int GetDelayWithPlayoutTimeStamp(int channel,
    WebRtc_UWord32 packet_timestamp, WebRtc_UWord32& delay) {
    return media_engine_voe_get_delay_with_playouttimestamp(
      _obj_ptr, channel, packet_timestamp, delay);
  }

  int GetNetworkStatistics(int channel, NetworkStatistics& stats) {
    return media_engine_voe_get_network_statistics(
      _obj_ptr, channel, stats);
  }

  // Stops or resumes playout and transmission on a temporary basis.
  int SetOnHoldStatus(int channel, bool enable,
    OnHoldModes mode = kHoldSendAndPlay) {
    return media_engine_voe_set_hold_status(_obj_ptr, channel, enable, mode);
  }

  // Gets the current playout and transmission status.
  int GetOnHoldStatus(int channel, bool& enabled, OnHoldModes& mode) {
    int ret_val = 0;
    int ret = media_engine_voe_get_hold_status(_obj_ptr, channel, enabled, ret_val);
    if (0 == ret) mode = (OnHoldModes) ret_val;
    return ret;
  }

  // Gets the last VoiceEngine error code.
  virtual int LastError() {
    return media_engine_voe_get_last_error(_obj_ptr);
  } 

  int EnablePacketCombine(int channel, bool enable, uint16_t max_packet) {
    return media_engine_voe_enable_data_combine(_obj_ptr, channel, enable, max_packet);
  }

  int EnablePacketSeparate(int channel, bool enable) {
    return media_engine_voe_enable_data_separate(_obj_ptr, channel, enable);
  }

private:
  media_engine_voice_base *_obj_ptr;
};

#endif // __MEDIA_ENGINE_VOE_HPP__
