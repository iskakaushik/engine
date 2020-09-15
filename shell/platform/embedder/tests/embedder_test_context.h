// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_TESTS_EMBEDDER_CONTEXT_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_TESTS_EMBEDDER_CONTEXT_H_

#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "flutter/fml/closure.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/mapping.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/tests/embedder_test_compositor.h"
#include "flutter/testing/elf_loader.h"
#include "flutter/testing/test_dart_native_resolver.h"
#include "flutter/testing/test_gl_surface.h"
#include "third_party/skia/include/core/SkImage.h"

namespace flutter {
namespace testing {

using SemanticsNodeCallback = std::function<void(const FlutterSemanticsNode*)>;
using SemanticsActionCallback =
    std::function<void(const FlutterSemanticsCustomAction*)>;

struct AOTDataDeleter {
  void operator()(FlutterEngineAOTData aot_data) {
    if (aot_data) {
      FlutterEngineCollectAOTData(aot_data);
    }
  }
};

using UniqueAOTData = std::unique_ptr<_FlutterEngineAOTData, AOTDataDeleter>;

class EmbedderTestContext {
 public:
  EmbedderTestContext(std::string assets_path = "");

  ~EmbedderTestContext();

  const std::string& GetAssetsPath() const;

  const fml::Mapping* GetVMSnapshotData() const;

  const fml::Mapping* GetVMSnapshotInstructions() const;

  const fml::Mapping* GetIsolateSnapshotData() const;

  const fml::Mapping* GetIsolateSnapshotInstructions() const;

  FlutterEngineAOTData GetAOTData() const;

  void SetRootSurfaceTransformation(SkMatrix matrix);

  void AddIsolateCreateCallback(fml::closure closure);

  void AddNativeCallback(const char* name, Dart_NativeFunction function);

  void SetSemanticsNodeCallback(
      const SemanticsNodeCallback& update_semantics_node);

  void SetSemanticsCustomActionCallback(
      const SemanticsActionCallback& semantics_custom_action);

  void SetPlatformMessageCallback(
      const std::function<void(const FlutterPlatformMessage*)>& callback);

  EmbedderTestCompositor& GetCompositor();

  std::future<sk_sp<SkImage>> GetNextSceneImage();

  size_t GetGLSurfacePresentCount() const;

  size_t GetSoftwareSurfacePresentCount() const;

  using GLGetFBOCallback = std::function<void(FlutterFrameInfo frame_info)>;

  //----------------------------------------------------------------------------
  /// @brief      Sets a callback that will be invoked (on the raster task
  ///             runner) when the engine asks the embedder for a new FBO ID at
  ///             the updated size.
  ///
  /// @attention  The callback will be invoked on the raster task runner. The
  ///             callback can be set on the tests host thread.
  ///
  /// @param[in]  callback  The callback to set. The previous callback will be
  ///                       un-registered.
  ///
  void SetGLGetFBOCallback(GLGetFBOCallback callback);

  uint32_t GetWindowFBOId() const;

  using GLPresentCallback = std::function<void(uint32_t fbo_id)>;

  //----------------------------------------------------------------------------
  /// @brief      Sets a callback that will be invoked (on the raster task
  ///             runner) when the engine presents an fbo that was given by the
  ///             embedder.
  ///
  /// @attention  The callback will be invoked on the raster task runner. The
  ///             callback can be set on the tests host thread.
  ///
  /// @param[in]  callback  The callback to set. The previous callback will be
  ///                       un-registered.
  ///
  void SetGLPresentCallback(GLPresentCallback callback);

  /// Sets the refresh rate of the display.
  void SetDisplayRefreshRate(double refresh_rate);

  /// Returns the last set refresh rate of the display. Returns zero otherwise.
  ///
  /// See: `SetDisplayRefreshRate`.
  double GetDisplayRefreshRate() const;

 private:
  // This allows the builder to access the hooks.
  friend class EmbedderConfigBuilder;

  using NextSceneCallback = std::function<void(sk_sp<SkImage> image)>;

  std::string assets_path_;
  ELFAOTSymbols aot_symbols_;
  std::unique_ptr<fml::Mapping> vm_snapshot_data_;
  std::unique_ptr<fml::Mapping> vm_snapshot_instructions_;
  std::unique_ptr<fml::Mapping> isolate_snapshot_data_;
  std::unique_ptr<fml::Mapping> isolate_snapshot_instructions_;
  UniqueAOTData aot_data_;
  std::vector<fml::closure> isolate_create_callbacks_;
  std::shared_ptr<TestDartNativeResolver> native_resolver_;
  SemanticsNodeCallback update_semantics_node_callback_;
  SemanticsActionCallback update_semantics_custom_action_callback_;
  std::function<void(const FlutterPlatformMessage*)> platform_message_callback_;
  std::unique_ptr<TestGLSurface> gl_surface_;
  std::unique_ptr<EmbedderTestCompositor> compositor_;
  NextSceneCallback next_scene_callback_;
  SkMatrix root_surface_transformation_;
  size_t gl_surface_present_count_ = 0;
  size_t software_surface_present_count_ = 0;
  std::mutex gl_callback_mutex_;
  GLGetFBOCallback gl_get_fbo_callback_;
  GLPresentCallback gl_present_callback_;
  double display_refresh_rate_ = 0;

  static VoidCallback GetIsolateCreateCallbackHook();

  static FlutterUpdateSemanticsNodeCallback
  GetUpdateSemanticsNodeCallbackHook();

  static FlutterUpdateSemanticsCustomActionCallback
  GetUpdateSemanticsCustomActionCallbackHook();

  static FlutterComputePlatformResolvedLocaleCallback
  GetComputePlatformResolvedLocaleCallbackHook();

  void SetupAOTMappingsIfNecessary();

  void SetupAOTDataIfNecessary();

  void SetupCompositor();

  void FireIsolateCreateCallbacks();

  void SetNativeResolver();

  void SetupOpenGLSurface(SkISize surface_size);

  bool GLMakeCurrent();

  bool GLClearCurrent();

  bool GLPresent(uint32_t fbo_id);

  uint32_t GLGetFramebuffer(FlutterFrameInfo frame_info);

  bool GLMakeResourceCurrent();

  void* GLGetProcAddress(const char* name);

  FlutterTransformation GetRootSurfaceTransformation();

  void PlatformMessageCallback(const FlutterPlatformMessage* message);

  bool SofwarePresent(sk_sp<SkImage> image);

  void FireRootSurfacePresentCallbackIfPresent(
      const std::function<sk_sp<SkImage>(void)>& image_callback);

  void SetNextSceneCallback(const NextSceneCallback& next_scene_callback);

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderTestContext);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_TESTS_EMBEDDER_CONTEXT_H_
