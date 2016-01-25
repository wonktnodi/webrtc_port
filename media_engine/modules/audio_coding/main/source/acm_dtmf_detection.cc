/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/audio_coding/main/source/acm_dtmf_detection.h"

#include "modules/audio_coding/main/interface/audio_coding_module_typedefs.h"

namespace webrtc {

ACMDTMFDetection::ACMDTMFDetection() {}

ACMDTMFDetection::~ACMDTMFDetection() {}

WebRtc_Word16 ACMDTMFDetection::Enable(ACMCountries /* cpt */) {
  return -1;
}

WebRtc_Word16 ACMDTMFDetection::Disable() {
  return -1;
}

WebRtc_Word16 ACMDTMFDetection::Detect(
    const WebRtc_Word16* /* in_audio_buff */,
    const WebRtc_UWord16 /* in_buff_len_word16 */,
    const WebRtc_Word32 /* in_freq_hz */,
    bool& /* tone_detected */,
    WebRtc_Word16& /* tone  */) {
  return -1;
}

}  // namespace webrtc
