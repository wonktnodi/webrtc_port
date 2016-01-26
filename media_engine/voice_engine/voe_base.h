#ifndef __MEDIA_ENGINE_VOE_BASE_H__
#define __MEDIA_ENGINE_VOE_BASE_H__

#include "common_types.h"

namespace webrtc {
class SharedData;

// VoiceEngineObserver
class VoiceEngineObserver {
public:
  // This method will be called after the occurrence of any runtime error
  // code, or warning notification, when the observer interface has been
  // installed using VoEBase::RegisterVoiceEngineObserver().
  virtual void CallbackOnError(const int channel, const int errCode) = 0;

protected:
  virtual ~VoiceEngineObserver() {}
};

}

using namespace webrtc;

const int kVoEDefault = -1;



class VoEBaseImpl {
public:
  // Retrieves the maximum number of channels that can be created.
  virtual int MaxNumOfChannels();

  // Creates a new channel and allocates the required resources for it.
  virtual int CreateChannel();

  // Deletes an existing channel and releases the utilized resources.
  virtual int DeleteChannel(int channel);

  // Starts sending packets to an already specified IP address and
  // port number for a specified |channel|.
  virtual int StartSend(int channel);

  // Stops sending packets from a specified |channel|.
  virtual int StopSend(int channel);
  
  // Get sending status from a specified |channel|.
  virtual int SendingStatus(int channel, bool &sending);

  // Starts forwarding the packets to the mixer/soundcard for a
  // specified |channel|.
  virtual int StartPlayout(int channel);

  // Stops forwarding the packets to the mixer/soundcard for a
  // specified |channel|.
  virtual int StopPlayout(int channel);

  // Prepares and initiates the VoiceEngine for reception of
  // incoming RTP/RTCP packets on the specified |channel|.
  virtual int StartReceive(int channel);

  // Stops receiving incoming RTP/RTCP packets on the specified |channel|.
  virtual int StopReceive(int channel);
  
  // Sets the NetEQ playout mode for a specified |channel| number.
  virtual int SetNetEQPlayoutMode(int channel, NetEqModes mode);

  // Gets the NetEQ playout mode for a specified |channel| number.
  virtual int GetNetEQPlayoutMode(int channel, NetEqModes& mode);

  // Sets the NetEQ background noise mode for a specified |channel| number.
  virtual int SetNetEQBGNMode(int channel, NetEqBgnModes mode);

  // Gets the NetEQ background noise mode for a specified |channel| number.
  virtual int GetNetEQBGNMode(int channel, NetEqBgnModes& mode);

  // Stops or resumes playout and transmission on a temporary basis.
  virtual int SetOnHoldStatus(int channel,
    bool enable,
    OnHoldModes mode = kHoldSendAndPlay);

  // Gets the current playout and transmission status.
  virtual int GetOnHoldStatus(int channel, bool& enabled, OnHoldModes& mode);


  virtual int GetDelayWithPlayoutTimeStamp(int channel, WebRtc_UWord32 timestamp,
    WebRtc_UWord32 &delay);

  virtual int GetNetworkStatistics(int channel, NetworkStatistics& stats);

  // Gets the version information for VoiceEngine and its components.
  virtual int GetVersion(char version[1024]);

  // Gets the last VoiceEngine error code.
  virtual int LastError();

  int EnablePacketCombine(int channel, bool enable, uint16_t max_packet);
  int EnablePacketSeparate(int channel, bool enable);

private:
  WebRtc_Word32 StartPlayout();
  WebRtc_Word32 StopPlayout();
  WebRtc_Word32 StartSend();
  WebRtc_Word32 StopSend();
  WebRtc_Word32 TerminateInternal();

  WebRtc_Word32 AddBuildInfo(char* str) const;
  WebRtc_Word32 AddVoEVersion(char* str) const;
public:
  VoEBaseImpl(SharedData*);
  virtual ~VoEBaseImpl();

private:  
  SharedData* _shared;
};




#endif // __MEDIA_ENGINE_VOE_BASE_H__
