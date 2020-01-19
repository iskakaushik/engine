// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_session.h"

#include <lib/trace/event.h>
#include <zircon/status.h>
#include <zircon/types.h>

#include <ostream>

#include "flutter/shell/platform/fuchsia/utils/logging.h"

namespace flutter_runner {
namespace {

/// Returns the system time at which the next frame is likely to be presented.
///
/// Consider the following scenarios, where in both the
/// scenarious the result will be the same.
///
/// Scenario 1:
/// presentation_interval is 2
/// ^     ^     ^     ^     ^
/// +     +     +     +     +
/// 0--1--2--3--4--5--6--7--8--9--
/// +        +  +
/// |        |  +---------> result: next_presentation_time
/// |        v
/// v        now
/// last_presentation_time
///
/// Scenario 2:
/// presentation_interval is 2
/// ^     ^     ^     ^     ^
/// +     +     +     +     +
/// 0--1--2--3--4--5--6--7--8--9--
///       +  +  +
///       |  |  +--------->result: next_presentation_time
///       |  |
///       |  +>now
///       |
///       +->last_presentation_time
std::pair<uint64_t, uint64_t> SnapToNextPhase(
    const uint64_t last_presentation_time,
    const uint64_t presentation_interval) {
  const uint64_t now = FlutterEngineGetCurrentTime();
  if (last_presentation_time >= now) {
    FX_LOGF(
        ERROR, FX_LOG_TAG,
        "Last frame was presented in the future (%u). Clamping to now (%u).",
        last_presentation_time, now);
    return std::make_pair(now, now + presentation_interval);
  }

  const uint64_t time_since_last_presentation = now - last_presentation_time;
  if (time_since_last_presentation < presentation_interval) {
    // This will be the most likely scenario if we are rendering at a good
    // frame-rate; short circuiting the other checks in this case.
    return std::make_pair(time_since_last_presentation,
                          time_since_last_presentation + presentation_interval);
  } else {
    const uint64_t num_phases_passed =
        (time_since_last_presentation / presentation_interval);
    const uint64_t predicted_presentation_time =
        last_presentation_time +
        (presentation_interval * (num_phases_passed + 1));

    return std::make_pair(predicted_presentation_time - presentation_interval,
                          predicted_presentation_time);
  }
}

}  // end namespace

ScenicSession::ScenicSession(const std::string& debug_label,
                             SessionEventCallback session_event_callback,
                             Renderer::ErrorCallback error_callback,
                             fuchsia::ui::scenic::ScenicPtr scenic,
                             async_dispatcher_t* input_dispatcher,
                             async_dispatcher_t* render_dispatcher)
    : session_(
          scenic::CreateScenicSessionPtrAndListenerRequest(scenic.get(),
                                                           render_dispatcher),
          input_dispatcher) {
  session_.set_error_handler([error_callback](zx_status_t status) {
    FX_LOGF(ERROR, FX_LOG_TAG, "Interface error %s on %s",
            zx_status_get_string(status), "fuchsia::ui::scenic::Session");
    error_callback();
  });
  session_.set_event_handler(session_event_callback);
  session_.SetDebugName(debug_label);
}

void ScenicSession::AwaitPresent(intptr_t baton) {
  // Only one |AwaitPresent| call can be outstanding at a time.
  FX_DCHECK(pending_present_baton_ == 0);

  pending_present_baton_ = baton;
}

void ScenicSession::QueuePresent() {
  TRACE_DURATION("gfx", "ScenicSession::QueuePresent");
  TRACE_FLOW_BEGIN("gfx", "ScenicSession::QueuePresent",
                   next_present_session_trace_id_);
  next_present_session_trace_id_++;

  // Throttle vsync if presentation callback is already pending. This allows
  // the paint tasks for this frame to execute in parallel with presentation
  // of last frame but still provides back-pressure to prevent us from
  // queuing even more work.
  if (presentation_callback_pending_) {
    present_session_pending_ = true;
  } else {
    Present();
  }
}

void ScenicSession::FireVsyncCallback(
    fuchsia::images::PresentationInfo presentation_info,
    intptr_t baton) {
  auto [previous_vsync, next_vsync] =
      SnapToNextPhase(presentation_info.presentation_time,
                      presentation_info.presentation_interval);

  FlutterEngineOnVsync(nullptr, baton, previous_vsync, next_vsync);
}

void ScenicSession::Present() {
  TRACE_DURATION("gfx", "ScenicSession::Present");
  while (processed_present_session_trace_id_ < next_present_session_trace_id_) {
    TRACE_FLOW_END("gfx", "ScenicSession::QueuePresent",
                   processed_present_session_trace_id_);
    processed_present_session_trace_id_++;
  }
  TRACE_FLOW_BEGIN("gfx", "Session::Present", next_present_trace_id_);
  next_present_trace_id_++;

  // Presentation callback is pending as a result of Present() call below.
  // Flush all session ops.
  presentation_callback_pending_ = true;
  session_.Present(0,  // presentation_time. (placeholder).
                   [this](fuchsia::images::PresentationInfo presentation_info) {
                     presentation_callback_pending_ = false;

                     // Process pending Present() calls.
                     if (present_session_pending_) {
                       present_session_pending_ = false;
                       Present();
                     }

                     // Notify Flutter of the vsync.
                     // TODO(dworsham): swap here?
                     intptr_t baton = pending_present_baton_;
                     pending_present_baton_ = 0;  // Clear before |OnVsync| to
                                                  // avoid races
                     if (baton != 0) {
                       FireVsyncCallback(std::move(presentation_info), baton);
                     }
                   }  // callback
  );
}

}  // namespace flutter_runner
