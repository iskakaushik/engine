// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RENDERER_SCENIC_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RENDERER_SCENIC_WINDOW_H_

#include <fuchsia/sys/cpp/fidl.h>
#include <fuchsia/ui/app/cpp/fidl.h>
#include <lib/async/dispatcher.h>
#include <lib/fidl/cpp/binding.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/vfs/cpp/composed_service_dir.h>
#include <lib/zx/eventpair.h>

#include <cstdint>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_input_handler.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_session.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_view.h"

namespace flutter_runner {

// This embedder component is responsible for interacting with the Scenic
// compositor on Fuchsia.  It encapsulates all of the state required to manage
// a single Scenic |Session| and |View|.
//
// The internal structure of the |ScenicWindow| is broken into 3 general parts:
//  + |ScenicSession| manages the Session lifetime and the timing of when frames
//    are presented to the compositor.
//  + |ScenicViewController| manages the various Scenic resources that comprise
//    the rendered scene.  It operates on the GPU thread.
//  + |ScenicInputHandler| manages the |SessionListener|, which allows it to
//    respond to various Scenic events for input and rendering.  It operates on
//    the platform thread.  Some of the events handled by the this object are
//    related to the rendered scene.  Those are dispatched to the GPU thread
//    for processing by the |ScenicViewController|.  As a consequence of
//    handling input events, this object is also responsible for managing IME
//    and a11y state for its bound instance of the Flutter engine.
class ScenicWindow final : public fuchsia::ui::app::ViewProvider,
                           public Renderer {
 public:
  ScenicWindow(Context context);
  ScenicWindow(const ScenicWindow&) = delete;
  ScenicWindow(ScenicWindow&&) = delete;
  ~ScenicWindow() = default;

  ScenicWindow& operator=(const ScenicWindow&) = delete;
  ScenicWindow& operator=(ScenicWindow&&) = delete;

  // |Renderer|
  void BindServices(BindServiceCallback bind_service_callback) override;
  void ConfigureCurrentIsolate() override;

  // |Renderer|
  void PlatformMessageResponse(const FlutterPlatformMessage* message) override;
  void UpdateSemanticsNode(const FlutterSemanticsNode* node) override;
  void UpdateSemanticsCustomAction(
      const FlutterSemanticsCustomAction* action) override;

  // |Renderer|
  void AwaitPresent(intptr_t baton) override;

  // |Renderer|
  bool CreateBackingStore(const FlutterBackingStoreConfig* layer_config,
                          FlutterBackingStore* backing_store_out) override;
  bool CollectBackingStore(const FlutterBackingStore* backing_store) override;
  bool PresentLayers(const FlutterLayer** layers, size_t layer_count) override;

 private:
  // |fuchsia::ui::app::ViewProvider|
  void CreateView(
      zx::eventpair token,
      fidl::InterfaceRequest<fuchsia::sys::ServiceProvider> incoming_services,
      fidl::InterfaceHandle<fuchsia::sys::ServiceProvider> outgoing_services)
      override;

  intptr_t pending_vsync_baton_ = 0;
  // ScenicSession session_;
  // ScenicView view_;
  // ScenicInputHandler input_handler_;

  fidl::Binding<fuchsia::ui::app::ViewProvider> view_provider_binding_;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RENDERER_SCENIC_WINDOW_H_
