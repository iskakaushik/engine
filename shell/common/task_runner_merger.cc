// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/task_runner_merger.h"

namespace flutter {

// TODO(iskakaushik): "-1" signals that unmerge has not been
// scheduled. Make it a constant or a flag.
TaskRunnerMerger::TaskRunnerMerger(TaskRunners task_runners)
    : task_queues_(fml::MessageLoopTaskQueues::GetInstance()),
      num_frames_till_unmerge_(-1) {
  platform_queue_id_ = task_runners.GetPlatformTaskRunner()->GetTaskQueueId();
  gpu_queue_id_ = task_runners.GetGPUTaskRunner()->GetTaskQueueId();
  is_merged_ = task_queues_->Owns(platform_queue_id_, gpu_queue_id_);
}

void TaskRunnerMerger::MergeGpuToPlatformAndResetUnmergeTimer(
    size_t frames_till_unmerge) {
  num_frames_till_unmerge_ = frames_till_unmerge;
  if (!is_merged_) {
    is_merged_ = task_queues_->Merge(platform_queue_id_, gpu_queue_id_);
  }
}

void TaskRunnerMerger::SubmitFrame() {
  if (!is_merged_) {
    return;
  }

  // we haven't been set to merge.
  if (num_frames_till_unmerge_ == -1) {
    return;
  }

  num_frames_till_unmerge_--;
  if (num_frames_till_unmerge_ == 0) {
    bool success = task_queues_->Unmerge(platform_queue_id_);
    is_merged_ = !success;
  }
}

}  // namespace flutter
