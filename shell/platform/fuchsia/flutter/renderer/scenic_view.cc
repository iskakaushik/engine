// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_view.h"

#include <lib/ui/scenic/cpp/commands.h>

#include "flutter/shell/platform/fuchsia/utils/logging.h"

namespace flutter_runner {

ScenicView::ScenicView(const std::string& debug_label,
                       ScenicSession& session,
                       fuchsia::ui::views::ViewToken view_token)
    : session_(session),
      root_view_(&session_.raw(), std::move(view_token), debug_label),
      root_node_(&session_.raw()) {
  root_node_.SetEventMask(fuchsia::ui::gfx::kMetricsEventMask);
  root_view_.AddChild(root_node_);
  session_.QueuePresent();
}

// fuchsia::ui::views::ViewRef ScenicView::view_ref() {
//   fuchsia::ui::views::ViewRef new_view_ref;
//   view_ref_pair_.view_ref.Clone(&new_view_ref);

//   return new_view_ref;
// }

bool ScenicView::CreateBackingStore(
    const FlutterBackingStoreConfig* layer_config,
    FlutterBackingStore* backing_store_out) {
  return false;
}

bool ScenicView::CollectBackingStore(const FlutterBackingStore* backing_store) {
  return false;
}

bool ScenicView::PresentLayers(const FlutterLayer** layers,
                               size_t layer_count) {
  return false;
}

void ScenicView::OnMetricsChanged(fuchsia::ui::gfx::Metrics new_metrics) {
  FX_DCHECK(false);
}

void ScenicView::OnChildViewConnected(scenic::ResourceId view_id) {
  FX_DCHECK(false);
}

void ScenicView::OnChildViewDisconnected(scenic::ResourceId view_id) {
  FX_DCHECK(false);
}

void ScenicView::OnChildViewStateChanged(scenic::ResourceId view_id,
                                         bool is_rendering) {
  FX_DCHECK(false);
}

void ScenicView::OnEnableWireframe(bool enable) {
  session_.raw().Enqueue(
      scenic::NewSetEnableDebugViewBoundsCmd(root_view_.id(), enable));
}

}  // namespace flutter_runner
