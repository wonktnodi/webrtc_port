#include <stdlib.h>
#include <string.h>  // memset

#include <sstream>
#include <string>
#include <vector>
#include <atlconv.h>

#include "gtest/gtest.h"

#include "modules/audio_data_segment/include/audio_data_combiner.h"
#include "modules/audio_data_segment/include/audio_data_separator.h"
#include "modules/rtp_rtcp/source/rtp_sender.h"
#include "testsupport/fileutils.h"
#include "data_dumper.h"

extern "C" void log_msg(const char * format, ... ) {
}

namespace webrtc {

WebRtc_Word32 BuildRTPheader(WebRtc_UWord8* dataBuffer,
                             const WebRtc_Word8 payloadType,
                             const bool markerBit,
                             const WebRtc_UWord32 captureTimeStamp,
                             const WebRtc_UWord16 sequenceNumber,
                             const WebRtc_UWord32 ssrc) {
  assert(payloadType >= 0);

  dataBuffer[0] = static_cast<WebRtc_UWord8>(0x80);  // version 2
  dataBuffer[1] = static_cast<WebRtc_UWord8>(payloadType);

  if (markerBit) {
    dataBuffer[1] |= kRtpMarkerBitMask;  // MarkerBit is set
  }

  ModuleRTPUtility::AssignUWord16ToBuffer(dataBuffer + 2, sequenceNumber);
  ModuleRTPUtility::AssignUWord32ToBuffer(dataBuffer + 4, captureTimeStamp);
  ModuleRTPUtility::AssignUWord32ToBuffer(dataBuffer + 8, ssrc);
  WebRtc_Word32 rtpHeaderLength = 12;

  return rtpHeaderLength;
}

class RefFiles {
public:
  RefFiles(const std::string &input_file, const std::string& output_file);
  ~RefFiles();

  void Open(const std::string &input_file, const std::string& output_file);
  void Close();

  void WriteRtpPacket(uint8_t payload_type, bool marker_bit, uint32_t timestamp,
                      uint16_t sequence, uint32_t ssrc, const uint8_t *data, size_t data_size);
  void WriteOriginalRtpPacket(uint8_t payload_type, bool marker_bit,
                              uint32_t timestamp,
                              uint16_t sequence, uint32_t ssrc, const uint8_t *data, size_t data_size);

  void WriteToFile(const uint8_t *data, size_t length);
  void WriteToOriginalFile(const uint8_t *data, size_t length);

  void CompareFiles();

  FILE* _orignal_fp;
  FILE* _output_fp;
  std::string _input_file;
  std::string _output_file;
};

RefFiles::RefFiles(const std::string &input_file,
                   const std::string& output_file) : _orignal_fp(0), _output_fp(0) {
  if (!input_file.empty()) {
    _orignal_fp = fopen(input_file.c_str(), "wb");
    _input_file = input_file;
    EXPECT_TRUE(_orignal_fp != NULL);
  }

  if (!output_file.empty()) {
    _output_fp = fopen(output_file.c_str(), "wb");
    _output_file = output_file;
    EXPECT_TRUE(_output_fp != NULL);
  }
}

RefFiles::~RefFiles() {
  Close();
}

void RefFiles::Open(const std::string &input_file,
                    const std::string& output_file) {
  if (!input_file.empty()) {
    _orignal_fp = fopen(input_file.c_str(), "wb");
    EXPECT_TRUE(_orignal_fp != NULL);
  }

  if (!output_file.empty()) {
    _output_fp = fopen(output_file.c_str(), "wb");
    EXPECT_TRUE(_output_fp != NULL);
  }
}

void RefFiles::Close() {
  if (_orignal_fp) {
    //EXPECT_EQ(EOF, fgetc(_orignal_fp));  // Make sure that we reached the end.
    fclose(_orignal_fp);
    _orignal_fp = 0;
  }

  if (_output_fp) {
    fclose(_output_fp);
    _output_fp = 0;
  }
}

void RefFiles::WriteRtpPacket(uint8_t payload_type, bool marker_bit,
                              uint32_t timestamp, uint16_t sequence, uint32_t ssrc,
                              const uint8_t *data, size_t data_size) {
  WebRtc_UWord8 dataBuffer[IP_PACKET_SIZE];
  uint32_t rtpHeaderLength = 0;
  rtpHeaderLength += BuildRTPheader(dataBuffer, payload_type,
                                    marker_bit, timestamp, sequence, ssrc);

  memcpy(dataBuffer + rtpHeaderLength, data, data_size);

  WriteToFile(dataBuffer, data_size + rtpHeaderLength);
}

void RefFiles::WriteToFile(const uint8_t *data, size_t length) {
  if (_output_fp) {
    size_t ret = fwrite(data, sizeof(uint8_t), length, _output_fp);
    ASSERT_EQ(length, ret);
  }
}

void RefFiles::WriteOriginalRtpPacket(uint8_t payload_type, bool marker_bit,
                                      uint32_t timestamp, uint16_t sequence, uint32_t ssrc,
                                      const uint8_t *data, size_t data_size) {
  WebRtc_UWord8 dataBuffer[IP_PACKET_SIZE];
  uint32_t rtpHeaderLength = 0;
  rtpHeaderLength += BuildRTPheader(dataBuffer, payload_type,
                                    marker_bit, timestamp, sequence, ssrc);

  memcpy(dataBuffer + rtpHeaderLength, data, data_size);

  WriteToOriginalFile(dataBuffer, data_size + rtpHeaderLength);
}

void RefFiles::WriteToOriginalFile(const uint8_t *data, size_t length) {
  if (_orignal_fp) {
    ASSERT_EQ(length, fwrite(data, sizeof(uint8_t), length, _orignal_fp));
  }
}

void RefFiles::CompareFiles() {
  Close();

  if (!_input_file.empty()) {
    _orignal_fp = fopen(_input_file.c_str(), "rb");
    EXPECT_TRUE(_orignal_fp != NULL);
  }

  if (!_output_file.empty()) {
    _output_fp = fopen(_output_file.c_str(), "rb");
    EXPECT_TRUE(_output_fp != NULL);
  }

  uint8_t src_buf[1024] = {0}, dest_buf[1024] = {0};

  while (!feof(_output_fp)) {
    ASSERT_NE(0u, fread(src_buf, sizeof(uint8_t), 1024, _orignal_fp));
    ASSERT_NE(0u, fread(dest_buf, sizeof(uint8_t), 1024, _output_fp));
    // Compare
    ASSERT_EQ(0, memcmp(dest_buf, src_buf, sizeof(uint8_t) * 1024));
  }

  Close();
}

class AudioDataSegmentTest : public
  ::testing::Test, /*public RTPSenderInterface,*/
   public AudioDataSeparatorCallback, public AudioDataCombinerCallback {
public:
  WebRtc_UWord32 GenerateRandom() {
    WebRtc_UWord32 ssrc = 0;

    do {
      ssrc = rand();
      ssrc = ssrc << 16;
      ssrc += rand();

    } while (ssrc == 0 || ssrc == 0xffffffff);

    return ssrc;
  }

  virtual WebRtc_UWord32 SSRC() const {
    return _ssrc;
  }

  virtual WebRtc_UWord32 Timestamp() const {
    return _timeStamp;
  }

  virtual WebRtc_Word32 BuildRTPheader(WebRtc_UWord8* dataBuffer,
                                       const WebRtc_Word8 payloadType,
                                       const bool markerBit,
                                       const WebRtc_UWord32 captureTimeStamp,
                                       const bool timeStampProvided = true,
                                       const bool incSequenceNumber = true) {
    assert(payloadType >= 0);

    dataBuffer[0] = static_cast<WebRtc_UWord8>(0x80);  // version 2
    dataBuffer[1] = static_cast<WebRtc_UWord8>(payloadType);

    if (markerBit) {
      dataBuffer[1] |= kRtpMarkerBitMask;  // MarkerBit is set
    }

    if (timeStampProvided) {
      _timeStamp = _startTimeStamp + captureTimeStamp;
    } else {
      // make a unique time stamp
      // we can't inc by the actual time, since then we increase the risk of back
      // timing.
      _timeStamp++;
    }

    ModuleRTPUtility::AssignUWord16ToBuffer(dataBuffer + 2, _sequenceNumber);
    ModuleRTPUtility::AssignUWord32ToBuffer(dataBuffer + 4, _timeStamp);
    ModuleRTPUtility::AssignUWord32ToBuffer(dataBuffer + 8, _ssrc);
    WebRtc_Word32 rtpHeaderLength = 12;

    if (incSequenceNumber) ++ _sequenceNumber;

    return rtpHeaderLength;
  }

  virtual WebRtc_UWord16 RTPHeaderLength() const {
    WebRtc_UWord16 rtpHeaderLength = 12;
    return rtpHeaderLength;
  }

  virtual WebRtc_UWord16 IncrementSequenceNumber() {
    return _sequenceNumber++;
  }
  virtual WebRtc_UWord16 SequenceNumber() const {
    return _sequenceNumber;
  }
  virtual WebRtc_UWord16 MaxPayloadLength() const {
    return 0;
  }
  virtual WebRtc_UWord16 MaxDataPayloadLength() const {
    return 0;
  }
  virtual WebRtc_UWord16 MaxDataPayloadLength() {
    return 0;
  }
  virtual WebRtc_UWord16 PacketOverHead() const {
    return 0;
  }
  virtual WebRtc_UWord16 ActualSendBitrateKbit() const {
    return 0;
  }

  virtual WebRtc_Word32 SendToNetwork(uint8_t* data_buffer,
                                      int payload_length,
                                      int rtp_header_length,
                                      int64_t capture_time_ms,
                                      StorageType storage) {
    if (0 == _data_separator) return 0;

    static uint32_t tick_cnt = 0;

    WebRtcRTPHeader rtp_hdr = {0};

    ModuleRTPUtility::RTPHeaderParser rtpParser(data_buffer,
        payload_length);

    ++ tick_cnt;
    rtpParser.Parse(rtp_hdr);
    _data_separator->ParseSegmentPacket(data_buffer + rtp_hdr.header.headerLength,
                                        payload_length, &rtp_hdr);
    return 0;
  }

  uint32_t SeparatedRtpCallback(const WebRtc_UWord8* payloadData,
                                const WebRtc_UWord16 payloadSize);

  uint32_t SendCombinedDataCallback(const WebRtc_UWord8* payloadData,
                                    const WebRtc_UWord16 payloadSize);

protected:
  AudioDataSegmentTest();

  virtual void SetUp();
  virtual void TearDown();

  void OpenDumpFile(const std::string &dump_file);

  void CheckDataCombiner(const std::string &dump_file);

  void TestPacketCombineAndSeparate();

  AudioDataSeparator *_data_separator;
  AudioDataCombiner *_data_combiner;

  data_reader_t *_dump_fp;

  WebRtc_UWord32  _ssrc;
  WebRtc_UWord32  _timeStamp;
  WebRtc_UWord32  _startTimeStamp;
  WebRtc_UWord16  _sequenceNumber;

  RefFiles _ref_files;
};

AudioDataSegmentTest::AudioDataSegmentTest() : _data_combiner(0),
  _data_separator(0), _dump_fp(0), _startTimeStamp(0), _sequenceNumber(0),
  _ssrc(0), _ref_files("f:/temp/input.rtp", "f:/temp/output.rtp")  {
  //_ssrc = GenerateRandom();
  _sequenceNumber = rand() / (RAND_MAX / MAX_INIT_RTP_SEQ_NUMBER);
}

void AudioDataSegmentTest::SetUp() {
  _data_combiner = new AudioDataCombiner(this);
  _data_combiner->RegisterCNPaylodType(13, 98, 99, 100);

  _data_separator = new AudioDataSeparator(this);
}

void AudioDataSegmentTest::TearDown() {
  if (_dump_fp) {
    ASSERT_EQ(0, dump_read_close(_dump_fp));
    _dump_fp = 0;
  }

  if (_data_combiner) {
    delete _data_combiner;
    _data_combiner = 0;
  }

  if (_data_separator) {
    delete _data_separator;
    _data_separator = 0;
  }
}

void AudioDataSegmentTest::OpenDumpFile(const std::string &dump_file) {
  USES_CONVERSION;
  ASSERT_EQ(0, dump_read_open(&_dump_fp, A2W(dump_file.c_str()), false, 0));
  ASSERT_TRUE(_dump_fp != NULL);
}

void AudioDataSegmentTest::CheckDataCombiner(const std::string &dump_file) {
  OpenDumpFile(dump_file);

  _data_combiner->SetMaxBufferedPacket(1);

  rd_buffer_t dmp_buf;

  while(dump_read(_dump_fp, &dmp_buf)) {
    ModuleRTPUtility::RTPHeaderParser rtpParser((WebRtc_UWord8*)dmp_buf.p.data,
        dmp_buf.p.hdr.plen);
    WebRtcRTPHeader rtp_header;
    rtpParser.Parse(rtp_header);

    if (0 == _ssrc) {
      _ssrc = rtp_header.header.ssrc;
      _data_combiner->SetSSRC(_ssrc);
      _data_combiner->SetSequenceNumber(rtp_header.header.sequenceNumber);
      _data_combiner->SetTimeStamp(rtp_header.header.timestamp);
    }

    const uint8_t *payload_data = (uint8_t*)dmp_buf.p.data +
                                  rtp_header.header.headerLength;
    const uint16_t payload_length = dmp_buf.p.hdr.plen -
                                    rtp_header.header.headerLength;
    _ref_files.WriteOriginalRtpPacket(rtp_header.header.payloadType,
                                      rtp_header.header.markerBit,
                                      rtp_header.header.timestamp,
                                      rtp_header.header.sequenceNumber, _ssrc,
                                      payload_data, payload_length);
    _data_combiner->SendPacket((uint8_t*)dmp_buf.p.data +
                               rtp_header.header.headerLength,
                               rtp_header.header.payloadType, rtp_header.header.timestamp,
                               rtp_header.header.sequenceNumber,
                               dmp_buf.p.hdr.plen - rtp_header.header.headerLength);
  }

  _data_combiner->SendPacketToTransport();
  _ref_files.CompareFiles();
}

uint32_t AudioDataSegmentTest::SeparatedRtpCallback(const WebRtc_UWord8*
    payloadData, const WebRtc_UWord16 payloadSize) {
  _ref_files.WriteToFile(payloadData, payloadSize);
  return payloadSize;
}

uint32_t AudioDataSegmentTest::SendCombinedDataCallback(
  const WebRtc_UWord8* payloadData, const WebRtc_UWord16 payloadSize) {
  WebRtcRTPHeader rtp_hdr = {0};

  static uint32_t tick_cnt = 0;
  ModuleRTPUtility::RTPHeaderParser rtpParser(payloadData,
      payloadSize);

  ++ tick_cnt;
  rtpParser.Parse(rtp_hdr);
  _data_separator->ParseSegmentPacket(payloadData + rtp_hdr.header.headerLength,
                                      payloadSize, &rtp_hdr);
  return 0;
}

void AudioDataSegmentTest::TestPacketCombineAndSeparate() {
  _data_combiner->SetMaxBufferedPacket(1);
  uint8_t test_buf[256] = {0};

  for(int i = 0; i < 5; ++ i) {
    memset(test_buf, '1' + i, 256);
    _ref_files.WriteOriginalRtpPacket(103, false, 10 + i, i + 1, SSRC(), test_buf,
                                      256);
    _data_combiner->SendPacket(test_buf, 103, 10 + i, i + 1, 256);
  }
}


TEST_F(AudioDataSegmentTest, TestDatacombining) {
  const std::string kInputRtpFile = webrtc::test::ProjectRootPath() +
                                    "resources/audio_data_combining.dat";
//#if defined(_MSC_VER) && (_MSC_VER >= 1700)
//  // For Visual Studio 2012 and later, we will have to use the generic reference
//  // file, rather than the windows-specific one.
//  const std::string kNetworkStatRefFile = webrtc::test::ProjectRootPath() +
//    "resources/neteq_network_stats.dat";
//#else
//  const std::string kNetworkStatRefFile =
//    webrtc::test::ResourcePath("neteq_network_stats", "dat");
//#endif
//  const std::string kRtcpStatRefFile =
//    webrtc::test::ResourcePath("neteq_rtcp_stats", "dat");
  CheckDataCombiner(kInputRtpFile);
  //TestPacketCombineAndSeparate();
}


}  // namespace webrtc