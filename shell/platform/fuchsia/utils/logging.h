// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_LOGGING_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_LOGGING_H_

#include <lib/syslog/global.h>

#include <cstdlib>
#include <iostream>

// TODO(dworsham): This doesn't work right
#if !defined(FX_LOG_TAG)
#define FX_LOG_TAG nullptr
#endif

namespace fx {

inline void Check(bool condition, const char* message) {
  if (!condition) {
    FX_LOG(FATAL, FX_LOG_TAG, message);
    std::cerr.flush();
    abort();
  }
}

#define FX_CHECKM(condition, msg) fx::Check(condition, msg)
#define FX_CHECK(condition) fx::Check(condition, #condition)

#ifndef NDEBUG
#define FX_DCHECKM(condition, msg) FX_CHECKM(condition, msg)
#define FX_DCHECK(condition) FX_CHECK(condition)
#else
#define FX_DCHECKM(condition)
#define FX_DCHECK(condition)
#endif

}  // namespace fx

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_LOGGING_H_
