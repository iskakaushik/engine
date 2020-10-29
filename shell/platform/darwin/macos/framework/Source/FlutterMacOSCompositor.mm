// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMacOSCompositor.h"

#import <OpenGL/gl.h>
#include "flutter/fml/logging.h"
#include "flutter/fml/platform/darwin/cf_utils.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/utils/mac/SkCGUtils.h"

namespace flutter {

FlutterMacOSCompositor::FlutterMacOSCompositor(SkISize surface_size,
                                               sk_sp<GrDirectContext> context,
                                               FlutterViewController* view_controller)
    : surface_size_(surface_size), context_(context), view_controller_(view_controller) {
  NSLog(@"\n init FlutterMacOSCompositor \n");
  FML_CHECK(!surface_size_.isEmpty()) << "Surface size must not be empty";
  FML_CHECK(context_);
}

FlutterMacOSCompositor::~FlutterMacOSCompositor() = default;

bool FlutterMacOSCompositor::CreateBackingStore(const FlutterBackingStoreConfig* config,
                                                FlutterBackingStore* backing_store_out) {
  return CreateFramebuffer(config, backing_store_out);
}

bool FlutterMacOSCompositor::CollectBackingStore(const FlutterBackingStore* backing_store) {
  // We have already set the destruction callback for the various backing
  // stores. Our user_data is just the canvas from that backing store and does
  // not need to be explicitly collected. Embedders might have some other state
  // they want to collect though.
  return true;
}

bool FlutterMacOSCompositor::Present(const FlutterLayer** layers, size_t layers_count) {
  present_callback_(layers, layers_count);
  return true;
}

bool FlutterMacOSCompositor::CreateGLRenderSurface(const FlutterBackingStoreConfig* config,
                                                   FlutterBackingStore* backing_store_out) {
  NSLog(@"CreateGLRenderSurface BEGIN");
  const auto image_info = SkImageInfo::MakeN32Premul(config->size.width, config->size.height);

  auto surface = SkSurface::MakeRenderTarget(context_.get(),               // context
                                             SkBudgeted::kNo,              // budgeted
                                             image_info,                   // image info
                                             1,                            // sample count
                                             kBottomLeft_GrSurfaceOrigin,  // surface origin
                                             nullptr,                      // surface properties
                                             false                         // mipmaps
  );

  if (!surface) {
    FML_LOG(ERROR) << "Could not create render target for compositor layer.";
    return false;
  }

  GrBackendTexture render_texture =
      surface->getBackendTexture(SkSurface::BackendHandleAccess::kDiscardWrite_BackendHandleAccess);

  if (!render_texture.isValid()) {
    FML_LOG(ERROR) << "Backend render texture was invalid.";
    return false;
  }

  GrGLTextureInfo texture_info = {};
  if (!render_texture.getGLTextureInfo(&texture_info)) {
    FML_LOG(ERROR) << "Could not access backend texture info.";
    return false;
  }

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->user_data = surface.get();
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeTexture;
  backing_store_out->open_gl.texture.target = texture_info.fTarget;
  backing_store_out->open_gl.texture.name = texture_info.fID;
  backing_store_out->open_gl.texture.format = texture_info.fFormat;
  // The balancing unref is in the destruction callback.
  surface->ref();
  backing_store_out->open_gl.texture.user_data = surface.get();
  backing_store_out->open_gl.texture.destruction_callback = [](void* user_data) {
    reinterpret_cast<SkSurface*>(user_data)->unref();
  };

  NSLog(@"CreateGLRenderSurface END");
  return true;
}

// This code is copied from gpu_surface_gl.cc / CreateOrUpdateSurfaces, WrapOnscreenSurface.
bool FlutterMacOSCompositor::CreateFramebuffer(const FlutterBackingStoreConfig* config,
                                               FlutterBackingStore* backing_store_out) {
  NSLog(@"CreateFramebuffer BEGIN");
  GrGLFramebufferInfo framebuffer_info = {};
  framebuffer_info.fFBOID = 0;
  framebuffer_info.fFormat = GL_RGBA8;
  const SkColorType color_type = kN32_SkColorType;

  GrBackendRenderTarget render_target(config->size.width,   // width
                                      config->size.height,  // height
                                      1,                    // sample count
                                      0,                    // stencil bits (TODO)
                                      framebuffer_info      // framebuffer info
  );

  sk_sp<SkColorSpace> colorspace = SkColorSpace::MakeSRGB();
  SkSurfaceProps surface_props(0, kUnknown_SkPixelGeometry);

  auto surface = SkSurface::MakeFromBackendRenderTarget(
      context_.get(),                                // gr context
      render_target,                                 // render target
      GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,  // origin
      color_type,                                    // color type
      colorspace,                                    // colorspace
      &surface_props                                 // surface properties
  );

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->user_data = surface.get();
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeFramebuffer;
  backing_store_out->open_gl.framebuffer.target = framebuffer_info.fFormat;
  backing_store_out->open_gl.framebuffer.name = framebuffer_info.fFBOID;
  // The balancing unref is in the destruction callback.
  surface->ref();
  backing_store_out->open_gl.framebuffer.user_data = surface.get();
  backing_store_out->open_gl.framebuffer.destruction_callback = [](void* user_data) {
    reinterpret_cast<SkSurface*>(user_data)->unref();
  };

  NSLog(@"CreateFramebuffer END");
  return true;
}

void FlutterMacOSCompositor::SetPresentCallback(
    const FlutterMacOSCompositor::PresentCallback& present_callback) {
  present_callback_ = present_callback;
}

}  // namespace flutter
