// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_SESSION_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_SESSION_H_

#include <fuchsia/images/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/async/dispatcher.h>
#include <lib/ui/scenic/cpp/session.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer.h"

namespace flutter_runner {

// This embedder component is responsible for managing the Scenic |Session|,
// especially the complexities of the |Present| callback.
//
// This class is used from multiple threads and has specific threading
// requirements.
//  + AwaitPresent() will be called from the UI thread.
//  + QueuePresent should only be called from the GPU thread.
//  + session() should only be called from the GPU thread.
//  + `pending_present_baton_` is shared between the UI and GPU threads, and is
//    thus locked behind an atomic.
class ScenicSession final {
 public:
  using SessionEventCallback =
      std::function<void(std::vector<fuchsia::ui::scenic::Event>)>;

  ScenicSession(const std::string& debug_label,
                SessionEventCallback session_event_callback,
                Renderer::ErrorCallback error_callback,
                fuchsia::ui::scenic::ScenicPtr scenic,
                async_dispatcher_t* input_dispatcher,
                async_dispatcher_t* render_dispatcher);
  ScenicSession(const ScenicSession&) = delete;
  ScenicSession(ScenicSession&&) = delete;
  ~ScenicSession() = default;

  ScenicSession& operator=(const ScenicSession&) = delete;
  ScenicSession& operator=(ScenicSession&&) = delete;

  scenic::Session& raw() { return session_; }

  // This method is called by the to framework to request that the session begin
  // an asynchronous wait on the next |Present| to complete.  The session will
  // call back into the engine at that time to indicate that a vsync occurred.
  //
  // This method is always called on the UI thread.
  void AwaitPresent(intptr_t baton);

  // Call this to request that a Present be performed as soon as the |session_|
  // permits it.
  void QueuePresent();

 private:
  void FireVsyncCallback(fuchsia::images::PresentationInfo presentation_info,
                         intptr_t baton);
  void Present();

  scenic::Session session_;

  // Flag given to the embedder in order to correlate VSyncs with |AwaitVsync|
  // requests.  Contention between the UI and GPU threads requires locking it
  // in an atomic.
  std::atomic<intptr_t> pending_present_baton_ = 0;

  // A flow event trace id for following |Session::Present| calls into
  // Scenic.  This will be incremented each |Session::Present| call.  By
  // convention, the Scenic side will also contain its own trace id that
  // begins at 0, and is incremented each |Session::Present| call.
  uint64_t next_present_trace_id_ = 0;
  uint64_t next_present_session_trace_id_ = 0;
  uint64_t processed_present_session_trace_id_ = 0;

  bool presentation_callback_pending_ = false;
  bool present_session_pending_ = false;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_SESSION_H_
