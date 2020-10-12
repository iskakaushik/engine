// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_METAL_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_METAL_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_delegate.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"

namespace flutter {

// forward declaration.
class IOSContextMetal;

class EmbedderSurfaceMetal final : public EmbedderSurface,
                                   public GPUSurfaceDelegate {
 public:
  EmbedderSurfaceMetal();

  ~EmbedderSurfaceMetal() = default;

 private:
  std::unique_ptr<IOSContextMetal> ios_context_;
  bool valid_ = false;
  sk_sp<SkSurface> sk_surface_;

  // |EmbedderSurface|
  bool IsValid() const override { return valid_; }

  // |EmbedderSurface|
  std::unique_ptr<Surface> CreateGPUSurface() override { return nullptr; }

  // |EmbedderSurface|
  sk_sp<GrDirectContext> CreateResourceContext() const override {
    return nullptr;
  }

  // |GPUSurfaceDelegate|
  ExternalViewEmbedder* GetExternalViewEmbedder() override { return nullptr; }

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderSurfaceMetal);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_METAL_H_
