// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_
#define FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_

#include <vector>
#include "flutter/flow/layers/layer.h"

namespace flutter {

class ContainerLayer : public Layer {
 public:
  ContainerLayer();
  ~ContainerLayer() override;

  void Add(std::shared_ptr<Layer> layer);

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

#if defined(OS_FUCHSIA)
  void UpdateScene(SceneUpdateContext& context) override;
#endif  // defined(OS_FUCHSIA)

  const std::vector<std::shared_ptr<Layer>>& layers() const { return layers_; }

  bool SubtreeHasPlatformViews() {
    return subtree_has_platform_views_;
  }

  void SetSubtreeHasPlatformViews(bool subtree_has_platform_views) {
    subtree_has_platform_views_ = subtree_has_platform_views;
  }

 protected:
  void PrerollChildren(PrerollContext* context,
                       const SkMatrix& child_matrix,
                       SkRect* child_paint_bounds);
  void PaintChildren(PaintContext& context) const;

#if defined(OS_FUCHSIA)
  void UpdateSceneChildren(SceneUpdateContext& context);
#endif  // defined(OS_FUCHSIA)

  // For OpacityLayer to restructure to have a single child.
  void ClearChildren() { layers_.clear(); }

 private:
  std::vector<std::shared_ptr<Layer>> layers_;
  bool subtree_has_platform_views_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(ContainerLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_CONTAINER_LAYER_H_
