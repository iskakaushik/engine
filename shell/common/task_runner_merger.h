// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_TASK_RUNNER_MERGER_H_
#define FLUTTER_SHELL_COMMON_TASK_RUNNER_MERGER_H_

#include "flutter/common/task_runners.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/message_loop_task_queues.h"

namespace flutter {

class TaskRunnerMerger : public fml::RefCountedThreadSafe<TaskRunnerMerger> {
 public:
  void MergeWithLease(size_t lease_term);

  void ExtendLease(size_t lease_term);

  void DecrementLease();

  bool AreMerged() const;

  TaskRunnerMerger(TaskRunners task_runners);

 private:
  fml::TaskQueueId platform_queue_id_;
  fml::TaskQueueId gpu_queue_id_;
  fml::RefPtr<fml::MessageLoopTaskQueues> task_queues_;
  std::atomic_int lease_term_;
  bool is_merged_;

  FML_FRIEND_REF_COUNTED_THREAD_SAFE(TaskRunnerMerger);
  FML_FRIEND_MAKE_REF_COUNTED(TaskRunnerMerger);
  FML_DISALLOW_COPY_AND_ASSIGN(TaskRunnerMerger);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_TASK_RUNNER_MERGER_H_
