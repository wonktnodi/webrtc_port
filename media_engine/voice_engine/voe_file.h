#ifndef __MEDIA_ENGINE_VOE_FILE_IMPL_H__
#define __MEDIA_ENGINE_VOE_FILE_IMPL_H__

#include "common_types.h"

namespace webrtc {

class SharedData;

}

using namespace webrtc;

class VoeFileImpl {
public:
  // Playout file locally

  virtual int StartPlayingFileLocally(
    int channel,
    const char fileNameUTF8[1024],
    bool loop = false,
    FileFormats format = kFileFormatPcm16kHzFile,
    float volumeScaling = 1.0,
    int startPointMs = 0,
    int stopPointMs = 0); 

  virtual int StopPlayingFileLocally(int channel);

  virtual int IsPlayingFileLocally(int channel);

  virtual int ScaleLocalFilePlayout(int channel, float scale);

  // Use file as microphone input

  virtual int StartPlayingFileAsMicrophone(
    int channel,
    const char fileNameUTF8[1024],
    bool loop = false ,
    bool mixWithMicrophone = false,
    FileFormats format = kFileFormatPcm16kHzFile,
    float volumeScaling = 1.0);
  
  virtual int StopPlayingFileAsMicrophone(int channel);

  virtual int IsPlayingFileAsMicrophone(int channel);

  virtual int ScaleFileAsMicrophonePlayout(int channel, float scale);

  // Record speaker signal to file

  virtual int StartRecordingPlayout(int channel,
    const char* fileNameUTF8,
    CodecInst* compression = NULL,
    int maxSizeBytes = -1); 

  virtual int StopRecordingPlayout(int channel);

  // Record microphone signal to file

  virtual int StartRecordingMicrophone(const char* fileNameUTF8,
    CodecInst* compression = NULL,
    int maxSizeBytes = -1);
    
  virtual int StopRecordingMicrophone();

public:
  VoeFileImpl(SharedData*);
  virtual ~VoeFileImpl();

private:
  SharedData *_shared;
};


#endif // __MEDIA_ENGINE_VOE_FILE_IMPL_H__
