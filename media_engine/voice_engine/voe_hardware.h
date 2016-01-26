#ifndef __MEDIA_ENGINE_VOE_HARDWARE_H__
#define __MEDIA_ENGINE_VOE_HARDWARE_H__

#include "shared_data.h"

using namespace webrtc;


class VoEHardwareImpl {
public:
  // Gets the number of audio devices available for recording.
  virtual int GetNumOfRecordingDevices(int& devices);

  // Gets the number of audio devices available for playout.
  virtual int GetNumOfPlayoutDevices(int& devices);

  // Gets the name of a specific recording device given by an |index|.
  // On Windows Vista/7, it also retrieves an additional unique ID
  // (GUID) for the recording device.
  virtual int GetRecordingDeviceName(int index, char strNameUTF8[128],
    char strGuidUTF8[128]);

  // Gets the name of a specific playout device given by an |index|.
  // On Windows Vista/7, it also retrieves an additional unique ID
  // (GUID) for the playout device.
  virtual int GetPlayoutDeviceName(int index, char strNameUTF8[128],
    char strGuidUTF8[128]);

  // Checks if the sound card is available to be opened for recording.
  virtual int GetRecordingDeviceStatus(bool& isAvailable);

  // Checks if the sound card is available to be opened for playout.
  virtual int GetPlayoutDeviceStatus(bool& isAvailable);

  // Sets the audio device used for recording.
  virtual int SetRecordingDevice(
    int index, StereoChannel recordingChannel = kStereoBoth);

  virtual int InputDevice(int &index);

  // Sets the audio device used for playout.
  virtual int SetPlayoutDevice(int index);  

  virtual int OutputDevice(int &index);

  VoEHardwareImpl(SharedData* shared);
  virtual ~VoEHardwareImpl();

private:  
  SharedData* _shared;
};

#endif // __MEDIA_ENGINE_VOE_HARDWARE_H__
