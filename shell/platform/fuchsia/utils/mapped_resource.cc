// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/utils/mapped_resource.h"

#include <fcntl.h>
#include <fuchsia/io/cpp/fidl.h>
#include <fuchsia/mem/cpp/fidl.h>
#include <lib/fdio/directory.h>
#include <lib/syslog/global.h>
#include <lib/trace/event.h>
#include <zircon/status.h>

#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/shell/platform/fuchsia/utils/vmo.h"

namespace fx {
namespace {

bool OpenVmoCommon(fuchsia::mem::Buffer* resource_vmo,
                   int dirfd,
                   const std::string& path,
                   bool executable) {
  // openat of a path with a leading '/' ignores the namespace fd.
  FX_CHECK(!path.empty() && path[0] != '/');

  bool result;
  if (dirfd == -1) {
    result = fx::VmoFromFilename(path, resource_vmo);
  } else {
    result = fx::VmoFromFilenameAt(dirfd, path, resource_vmo);
  }

  if (executable && result) {
    // VmoFromFilename(At) will return VMOs without ZX_RIGHT_EXECUTE, so we
    // need replace_as_executable to be able to map them as ZX_VM_PERM_EXECUTE.
    // TODO(mdempsky): Update comment once SEC-42 is fixed.
    zx_status_t status = resource_vmo->vmo.replace_as_executable(
        zx::handle(), &resource_vmo->vmo);
    if (status != ZX_OK) {
      FX_LOGF(ERROR, FX_LOG_TAG, "Failed to make VMO executable: %s",
              zx_status_get_string(status));
      result = false;
    }
  }
  if (dirfd != -1) {
    close(dirfd);
  }

  return result;
}

bool OpenVmo(fuchsia::mem::Buffer* resource_vmo,
             int dirfd,
             const std::string& path,
             bool executable) {
  TRACE_DURATION("dart", "OpenVmo", "path", path);
  FX_DCHECK(dirfd >= 0);

  return OpenVmoCommon(resource_vmo, dirfd, path, executable);
}

bool OpenVmo(fuchsia::mem::Buffer* resource_vmo,
             fdio_ns_t* fdio_namespace,
             const std::string& path,
             bool executable) {
  TRACE_DURATION("dart", "OpenVmo", "path", path);

  int root_dir = -1;
  if (fdio_namespace != nullptr) {
    auto root_dir = fdio_ns_opendir(fdio_namespace);
    if (root_dir < 0) {
      FX_LOG(ERROR, FX_LOG_TAG, "Failed to open namespace directory");
      return false;
    }
  }

  return OpenVmoCommon(resource_vmo, root_dir, path, executable);
}

bool MapVmo(const std::string& path,
            fuchsia::mem::Buffer resource_vmo,
            MappedResource& resource,
            bool executable) {
  TRACE_DURATION("dart", "MapVmo", "path", path);

  if (resource_vmo.size == 0) {
    resource = MappedResource();

    return true;
  }

  uint32_t flags = ZX_VM_PERM_READ;
  if (executable) {
    flags |= ZX_VM_PERM_EXECUTE;
  }

  uintptr_t addr;
  zx_status_t status = zx::vmar::root_self()->map(
      0, resource_vmo.vmo, 0, resource_vmo.size, flags, &addr);
  if (status != ZX_OK) {
    FX_LOGF(ERROR, FX_LOG_TAG, "Failed to map: %s",
            zx_status_get_string(status));
    resource = MappedResource();

    return false;
  }

  resource =
      MappedResource(reinterpret_cast<uint8_t*>(addr), resource_vmo.size);

  return true;
}

bool UnmapVmo(const void* address, size_t size) {
  zx_status_t status = zx::vmar::root_self()->unmap(
      reinterpret_cast<const uintptr_t>(address), size);
  if (status != ZX_OK) {
    FX_LOGF(ERROR, FX_LOG_TAG, "Failed to unmap: %s",
            zx_status_get_string(status));

    return false;
  }

  return true;
}

int OpenFdExec(const std::string& path, int dirfd) {
  int fd = -1;
  zx_status_t result = fdio_open_fd_at(
      dirfd, path.c_str(),
      fuchsia::io::OPEN_RIGHT_READABLE | fuchsia::io::OPEN_RIGHT_EXECUTABLE,
      &fd);
  if (result != ZX_OK) {
    FX_LOGF(ERROR, FX_LOG_TAG, "fdio_open_fd_at(%s) failed: %s", path.c_str(),
            zx_status_get_string(result));
    return -1;
  }
  return fd;
}

}  // namespace

ElfSnapshot::~ElfSnapshot() {
  Dart_UnloadELF(handle_);
}

bool ElfSnapshot::Load(fdio_ns_t* fdio_namespace, const std::string& path) {
  int root_dir = -1;
  if (fdio_namespace == nullptr) {
    root_dir = AT_FDCWD;
  } else {
    root_dir = fdio_ns_opendir(fdio_namespace);
    if (root_dir < 0) {
      FX_LOG(ERROR, FX_LOG_TAG, "Failed to open namespace directory");
      return false;
    }
  }
  return Load(root_dir, path);
}

bool ElfSnapshot::Load(int dirfd, const std::string& path) {
  const int fd = OpenFdExec(path, dirfd);
  if (fd < 0) {
    FX_LOGF(ERROR, FX_LOG_TAG, "Failed to open VMO for %s from dir.",
            path.c_str());
    return false;
  }
  return Load(fd);
}

bool ElfSnapshot::Load(int fd) {
  const char* error;
  handle_ = Dart_LoadELF_Fd(fd, 0, &error, &vm_data_, &vm_instrs_,
                            &isolate_data_, &isolate_instrs_);
  close(fd);

  if (handle_ == nullptr) {
    FX_LOGF(ERROR, FX_LOG_TAG, "Failed load ELF: %s", error);
  }
  return handle_ != nullptr;
}

MappedResource MappedResource::MakeFileMapping(int dirfd,
                                               const std::string& path,
                                               bool executable) {
  fx::MappedResource mapping;
  bool success =
      fx::MappedResource::LoadFromDir(dirfd, path, mapping, executable);
  if (!success) {
    FX_LOGF(ERROR, FX_LOG_TAG, "Failed creating MappedResource for %s",
            path.c_str());
  }

  return mapping;
}

MappedResource::~MappedResource() {
  if (address_ != nullptr && size_ != 0) {
    UnmapVmo(address_, size_);
  }
}

bool MappedResource::LoadFromNamespace(fdio_ns_t* fdio_namespace,
                                       const std::string& path,
                                       MappedResource& resource,
                                       bool executable) {
  TRACE_DURATION("dart", "LoadFromNamespace", "path", path);

  fuchsia::mem::Buffer resource_vmo;
  return OpenVmo(&resource_vmo, fdio_namespace, path, executable) &&
         MapVmo(path, std::move(resource_vmo), resource, executable);
}

bool MappedResource::LoadFromDir(int dirfd,
                                 const std::string& path,
                                 MappedResource& resource,
                                 bool executable) {
  TRACE_DURATION("dart", "LoadFromDir", "path", path);

  fuchsia::mem::Buffer resource_vmo;
  return OpenVmo(&resource_vmo, dirfd, path, executable) &&
         MapVmo(path, std::move(resource_vmo), resource, executable);
}

}  // namespace fx
