/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "test/test_suite.h"
#include <stdlib.h>

int main(int argc, char** argv) {
  webrtc::test::TestSuite test_suite(argc, argv);
  int ret = test_suite.Run();
  system("pause");
  return ret;
}