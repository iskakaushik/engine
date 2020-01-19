// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_RUNNER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_RUNNER_H_

#include <fuchsia/sys/cpp/fidl.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/sys/cpp/component_context.h>

#include <memory>

#include "flutter/shell/platform/fuchsia/utils/mapped_resource.h"

namespace dart_runner {

class DartRunner : public fuchsia::sys::Runner {
 public:
  explicit DartRunner();
  DartRunner(const DartRunner&) = delete;
  ~DartRunner() override;

  DartRunner& operator=(const DartRunner&) = delete;

 private:
  // |fuchsia::sys::Runner| implementation:
  void StartComponent(fuchsia::sys::Package package,
                      fuchsia::sys::StartupInfo startup_info,
                      fidl::InterfaceRequest<fuchsia::sys::ComponentController>
                          controller) override;

  std::unique_ptr<sys::ComponentContext> context_;
  fidl::BindingSet<fuchsia::sys::Runner> bindings_;

#if !defined(AOT_RUNTIME)
  fx::MappedResource vm_snapshot_data_;
  fx::MappedResource vm_snapshot_instructions_;
#endif
};

}  // namespace dart_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_RUNNER_H_
