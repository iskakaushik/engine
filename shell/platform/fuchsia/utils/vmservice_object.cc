// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/utils/vmservice_object.h"

#include <dirent.h>
#include <fuchsia/io/cpp/fidl.h>
#include <lib/syslog/global.h>
#include <zircon/status.h>

#include <cstdlib>
#include <string>

#include "flutter/shell/platform/fuchsia/utils/logging.h"

namespace {

bool ReadDirContents(const std::string& path, std::vector<std::string>* out) {
  out->clear();
  DIR* dir = opendir(path.c_str());
  if (!dir) {
    return false;
  }
  struct dirent* de;
  errno = 0;
  while ((de = readdir(dir)) != nullptr) {
    out->push_back(de->d_name);
  }
  closedir(dir);
  return !errno;
}

}  // namespace

namespace fx {

void VMServiceObject::GetContents(LazyEntryVector* out_vector) const {
  // List /tmp/dart.services if it exists, and push its contents as
  // as the conrtents of this directory.
  std::vector<std::string> files;
  if (!ReadDirContents(kPortDir, &files)) {
    FX_LOGF(ERROR, nullptr,
            "Failed to read Dart VM Service port directory '%s': %s", kPortDir,
            strerror(errno));
    return;
  }
  for (const auto& file : files) {
    if ((file == ".") || (file == "..")) {
      continue;
    }
    out_vector->push_back({std::stoul(file) + GetStartingId(), file,
                           fuchsia::io::MODE_TYPE_FILE});
  }
}

zx_status_t VMServiceObject::GetFile(Node** out_node,
                                     uint64_t id,
                                     std::string name) const {
  return ZX_ERR_NOT_FOUND;
}

}  // namespace fx
