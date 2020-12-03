// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/tests/embedder_test_context_metal.h"

#include <memory>

namespace flutter {
namespace testing {

EmbedderTestContextMetal::EmbedderTestContextMetal(std::string assets_path)
    : EmbedderTestContext(assets_path),
      metal_context_(std::make_unique<TestMetalContext>()) {}

EmbedderTestContextMetal::~EmbedderTestContextMetal() {}

void EmbedderTestContextMetal::SetupSurface(SkISize surface_size) {
  FML_CHECK(!metal_surface_);
  metal_surface_ =
      TestMetalSurface::Create(*metal_context_.get(), surface_size);
}

size_t EmbedderTestContextMetal::GetSurfacePresentCount() const {
  return 0;
}

void EmbedderTestContextMetal::SetupCompositor() {
  FML_CHECK(false) << "Compositor rendering not supported in metal.";
}

TestMetalContext* EmbedderTestContextMetal::GetTestMetalContext() {
  return metal_context_.get();
}

}  // namespace testing
}  // namespace flutter
