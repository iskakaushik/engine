// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_window.h"

#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/ui/scenic/cpp/id.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>
#include <zircon/status.h>

#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/third_party/tonic/converter/dart_converter.h"
#include "flutter/third_party/tonic/logging/dart_error.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace flutter_runner {
namespace {

// ScenicInputHandler::RenderDispatchTable GetRenderDispatchTable(
//     ScenicView& view,
//     async_dispatcher_t* render_dispatcher) {
//   return {
//       .metrics_changed_callback =
//           [&view, render_dispatcher](fuchsia::ui::gfx::Metrics new_metrics) {
//             async::PostTask(render_dispatcher, [&view, new_metrics]() {
//               view.OnMetricsChanged(new_metrics);
//             });
//           },
//       .child_view_connected_callback =
//           [&view, render_dispatcher](scenic::ResourceId view_holder_id) {
//             async::PostTask(render_dispatcher, [&view, view_holder_id]() {
//               view.OnChildViewConnected(view_holder_id);
//             });
//           },
//       .child_view_disconnected_callback =
//           [&view, render_dispatcher](scenic::ResourceId view_holder_id) {
//             async::PostTask(render_dispatcher, [&view, view_holder_id]() {
//               view.OnChildViewDisconnected(view_holder_id);
//             });
//           },
//       .child_view_state_changed_callback =
//           [&view, render_dispatcher](scenic::ResourceId view_holder_id,
//                                      bool is_rendering) {
//             async::PostTask(
//                 render_dispatcher, [&view, view_holder_id, is_rendering]() {
//                   view.OnChildViewStateChanged(view_holder_id, is_rendering);
//                 });
//           },
//       .view_enable_wireframe_callback =
//           [&view, render_dispatcher](bool enable) {
//             async::PostTask(render_dispatcher, [&view, enable]() {
//               view.OnEnableWireframe(enable);
//             });
//           },
//   };
// }

}  // namespace

ScenicWindow::ScenicWindow(Context context) : view_provider_binding_(this) {
  view_provider_binding_.set_error_handler(
      [error_callback = context.error_callback](zx_status_t status) {
        FX_LOGF(
            ERROR, FX_LOG_TAG,
            "Interface error (binding) for fuchsia::ui::app::ViewProviders: %s",
            zx_status_get_string(status));
        error_callback();
      });
}

// session_(
//           context.debug_label,
//           std::bind(&ScenicInputHandler::OnScenicEvent,
//                     nullptr,  //&input_handler_,
//                     std::placeholders::_1),
//           context.error_callback,
//           context.incoming_services->Connect<fuchsia::ui::scenic::Scenic>(),
//           context.input_dispatcher,
//           context.render_dispatcher)

// input_handler_(GetRenderDispatchTable(view_, context_.render_dispatcher),
//                context_.error_callback,
//                context_.incoming_services) {}

void ScenicWindow::BindServices(BindServiceCallback bind_service_callback) {
  fidl::InterfaceRequestHandler<fuchsia::ui::app::ViewProvider> view_handler =
      [this](fidl::InterfaceRequest<fuchsia::ui::app::ViewProvider> request) {
        FX_DCHECK(!view_provider_binding_.is_bound());
        view_provider_binding_.Bind(std::move(request));
      };
  bind_service_callback(
      fuchsia::ui::app::ViewProvider::Name_,
      std::make_unique<vfs::Service>(std::move(view_handler)));
}

void ScenicWindow::ConfigureCurrentIsolate() {
  Dart_Handle library = Dart_LookupLibrary(tonic::ToDart("dart:fuchsia"));
  FX_CHECK(!tonic::LogIfError(library));

  (void)library;
  // TODO(dworsham)
  // result = Dart_SetField(library, tonic::ToDart("_viewRef"),
  //                        tonic::ToDart(zircon::dart::Handle::Create(
  //                            std::move(view_ref.reference))));
  // FX_CHECK(!tonic::LogIfError(result));
}

void ScenicWindow::PlatformMessageResponse(
    const FlutterPlatformMessage* message) {
  FX_DCHECK(false);
}

void ScenicWindow::UpdateSemanticsNode(const FlutterSemanticsNode* node) {
  FX_DCHECK(false);
}

void ScenicWindow::UpdateSemanticsCustomAction(
    const FlutterSemanticsCustomAction* action) {
  FX_DCHECK(false);
}

void ScenicWindow::AwaitPresent(intptr_t baton) {
  FX_LOGF(ERROR, FX_LOG_TAG, "AwaitPresent: %i", baton);
  pending_vsync_baton_ = baton;
  // session_->AwaitPresent(baton);
}

bool ScenicWindow::CreateBackingStore(
    const FlutterBackingStoreConfig* layer_config,
    FlutterBackingStore* backing_store_out) {
  // FX_DCHECK(view_.has_value());

  // return view_->CreateBackingStore(layer_config, backing_store_out);
  return false;
}

bool ScenicWindow::CollectBackingStore(
    const FlutterBackingStore* backing_store) {
  // FX_DCHECK(view_.has_value());

  // return view_->CollectBackingStore(backing_store);
  return false;
}

bool ScenicWindow::PresentLayers(const FlutterLayer** layers,
                                 size_t layer_count) {
  // FX_DCHECK(view_.has_value());

  // return view_->PresentLayers(layers, layer_count);
  return false;
}

void ScenicWindow::CreateView(
    zx::eventpair token,
    fidl::InterfaceRequest<
        fuchsia::sys::ServiceProvider> /* incoming_services */,
    fidl::InterfaceHandle<
        fuchsia::sys::ServiceProvider> /* outgoing_services */) {
  // FX_DCHECK(!root_view_.has_value());

  // root_view_.emplace(&session_.raw(), scenic::ToViewToken(std::move(token)),
  //                    std::move(view_ref_pair_.control_ref), view_ref(),
  //                    "FlutterView");
  // session_.QueuePresent();
}

}  // namespace flutter_runner
