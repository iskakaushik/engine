// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_PLATFORM_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_PLATFORM_VIEW_H_

#include <functional>

#include "flutter/fml/macros.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"
#include "flutter/shell/platform/embedder/embedder_surface_software.h"
#include "flutter/shell/platform/embedder/embedder_vsync_waiter.h"

#if SHELL_ENABLE_GL
#include "flutter/shell/platform/embedder/embedder_surface_gl.h"
#endif  // SHELL_ENABLE_GL
#if SHELL_ENABLE_VULKAN
#include "flutter/shell/platform/embedder/embedder_surface_vulkan.h"
#endif  // SHELL_ENABLE_VULKAN

namespace flutter {

class EmbedderPlatformView final : public PlatformView {
 public:
  using UpdateSemanticsNodesCallback =
      std::function<void(flutter::SemanticsNodeUpdates update)>;
  using UpdateSemanticsCustomActionsCallback =
      std::function<void(flutter::CustomAccessibilityActionUpdates actions)>;
  using PlatformMessageResponseCallback =
      std::function<void(fml::RefPtr<flutter::PlatformMessage>)>;

  struct PlatformDispatchTable {
    UpdateSemanticsNodesCallback update_semantics_nodes_callback;  // optional
    UpdateSemanticsCustomActionsCallback
        update_semantics_custom_actions_callback;  // optional
    PlatformMessageResponseCallback
        platform_message_response_callback;             // optional
    EmbedderVsyncWaiter::VsyncCallback vsync_callback;  // optional
  };

#if SHELL_ENABLE_GL
  // Creates a platform view that sets up an OpenGL rasterizer.
  EmbedderPlatformView(
      PlatformView::Delegate& delegate,
      flutter::TaskRunners task_runners,
      EmbedderSurfaceGL::GLDispatchTable gl_dispatch_table,
      bool fbo_reset_after_present,
      PlatformDispatchTable platform_dispatch_table,
      std::unique_ptr<EmbedderExternalViewEmbedder> external_view_embedder);
#endif
#if SHELL_ENABLE_VULKAN
  // Create a platform view that sets up a Vulkan rasterizer.
  EmbedderPlatformView(
      PlatformView::Delegate& delegate,
      flutter::TaskRunners task_runners,
      EmbedderSurfaceVulkan::VulkanDispatchTable vulkan_dispatch_table,
      PlatformDispatchTable platform_dispatch_table,
      std::unique_ptr<EmbedderExternalViewEmbedder> external_view_embedder);
#endif
  // Create a platform view that sets up a software rasterizer.
  EmbedderPlatformView(
      PlatformView::Delegate& delegate,
      flutter::TaskRunners task_runners,
      EmbedderSurfaceSoftware::SoftwareDispatchTable software_dispatch_table,
      PlatformDispatchTable platform_dispatch_table,
      std::unique_ptr<EmbedderExternalViewEmbedder> external_view_embedder);
  ~EmbedderPlatformView() override;

  // |PlatformView|
  void UpdateSemantics(
      flutter::SemanticsNodeUpdates update,
      flutter::CustomAccessibilityActionUpdates actions) override;

  // |PlatformView|
  void HandlePlatformMessage(
      fml::RefPtr<flutter::PlatformMessage> message) override;

 private:
  std::unique_ptr<EmbedderSurface> embedder_surface_;
  PlatformDispatchTable platform_dispatch_table_;

  // |PlatformView|
  std::unique_ptr<Surface> CreateRenderingSurface() override;

  // |PlatformView|
  sk_sp<GrContext> CreateResourceContext() const override;

  // |PlatformView|
  std::unique_ptr<VsyncWaiter> CreateVSyncWaiter() override;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderPlatformView);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_PLATFORM_VIEW_H_
