// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/test_metal_surface_impl.h"

#include <Metal/Metal.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/testing/test_metal_context.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace flutter {

TestMetalSurfaceImpl::TestMetalSurfaceImpl(TestMetalContext& test_metal_context,
                                           const SkISize& surface_size)
    : test_metal_context_(test_metal_context) {
  if (surface_size.isEmpty()) {
    FML_LOG(ERROR) << "Size of test Metal surface was empty.";
    return;
  }

  auto texture_descriptor = fml::scoped_nsobject{
      [[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                          width:surface_size.width()
                                                         height:surface_size.height()
                                                      mipmapped:NO] retain]};

  // The most pessimistic option and disables all optimizations but allows tests
  // the most flexible access to the surface. They may read and write to the
  // surface from shaders or use as a pixel view.
  texture_descriptor.get().usage = MTLTextureUsageUnknown;

  if (!texture_descriptor) {
    FML_LOG(ERROR) << "Invalid texture descriptor.";
    return;
  }

  TestMetalContext::TextureInfo texture_info = test_metal_context_.CreateMetalTexture(surface_size);
  id<MTLTexture> texture = (__bridge id<MTLTexture>)texture_info.texture;
  GrMtlTextureInfo skia_texture_info;
  skia_texture_info.fTexture = sk_cf_obj<const void*>{[texture retain]};

  auto backend_render_target = GrBackendRenderTarget{
      surface_size.width(),   // width
      surface_size.height(),  // height
      1,                      // sample count
      skia_texture_info       // texture info
  };

  auto surface = SkSurface::MakeFromBackendRenderTarget(
      test_metal_context_.GetSkiaContext().get(),  // context
      backend_render_target,                       // backend render target
      kTopLeft_GrSurfaceOrigin,                    // surface origin
      kBGRA_8888_SkColorType,                      // color type
      nullptr,                                     // color space
      nullptr,                                     // surface properties
      nullptr,  // release proc (texture is already ref counted in sk_cf_obj)
      nullptr   // release context
  );

  if (!surface) {
    FML_LOG(ERROR) << "Could not create Skia surface from a Metal texture.";
    return;
  }

  surface_ = std::move(surface);
  is_valid_ = true;
}

// |TestMetalSurface|
TestMetalSurfaceImpl::~TestMetalSurfaceImpl() = default;

// |TestMetalSurface|
bool TestMetalSurfaceImpl::IsValid() const {
  return is_valid_;
}
// |TestMetalSurface|
sk_sp<GrDirectContext> TestMetalSurfaceImpl::GetGrContext() const {
  return IsValid() ? test_metal_context_.GetSkiaContext() : nullptr;
}
// |TestMetalSurface|
sk_sp<SkSurface> TestMetalSurfaceImpl::GetSurface() const {
  return IsValid() ? surface_ : nullptr;
}

}  // namespace flutter
