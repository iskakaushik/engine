// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_VIEW_H_

#include <fuchsia/ui/gfx/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/ui/scenic/cpp/id.h>
#include <lib/ui/scenic/cpp/resources.h>
#include <lib/ui/scenic/cpp/view_ref_pair.h>

#include <optional>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_session.h"

namespace flutter_runner {

// The embedder component that is responsible for manipulating the embedders'
// Scenic |View| in response to new frames being created by the renderer.
//
// This component exists on the GPU thread.
class ScenicView final {
 public:
  ScenicView(const std::string& debug_label,
             ScenicSession& session,
             fuchsia::ui::views::ViewToken view_token);
  ~ScenicView() = default;

  ScenicView(const ScenicView&) = delete;
  ScenicView& operator=(const ScenicView&) = delete;

  // The Flutter engine invokes these methods via |FlutterCompositor| callbacks.
  // It calls them on the GPU thread.
  bool CreateBackingStore(const FlutterBackingStoreConfig* layer_config,
                          FlutterBackingStore* backing_store_out);
  bool CollectBackingStore(const FlutterBackingStore* backing_store);
  bool PresentLayers(const FlutterLayer** layers, size_t layer_count);

  // |ScenicInputHandler| invokes these callbacks.  It calls them on the GPU
  // thread.
  void OnMetricsChanged(fuchsia::ui::gfx::Metrics new_metrics);
  void OnChildViewConnected(scenic::ResourceId view_id);
  void OnChildViewDisconnected(scenic::ResourceId view_id);
  void OnChildViewStateChanged(scenic::ResourceId view_id, bool is_rendering);
  void OnEnableWireframe(bool enable);

 private:
  ScenicSession& session_;

  scenic::View root_view_;
  scenic::EntityNode root_node_;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_VIEW_H_
