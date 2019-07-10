// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include <atomic>
#include <thread>

#include "flutter/fml/message_loop.h"
#include "flutter/fml/synchronization/count_down_latch.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/task_runner.h"
#include "flutter/fml/task_runner_merger.h"
#include "gtest/gtest.h"

TEST(TaskRunnerMerger, RemainMergedTillLeaseExpires) {
  fml::MessageLoop* loop1 = nullptr;
  fml::AutoResetWaitableEvent latch1;
  fml::AutoResetWaitableEvent term1;
  std::thread thread1([&loop1, &latch1, &term1]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    loop1 = &fml::MessageLoop::GetCurrent();
    latch1.Signal();
    term1.Wait();
  });

  fml::MessageLoop* loop2 = nullptr;
  fml::AutoResetWaitableEvent latch2;
  fml::AutoResetWaitableEvent term2;
  std::thread thread2([&loop2, &latch2, &term2]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    loop2 = &fml::MessageLoop::GetCurrent();
    latch2.Signal();
    term2.Wait();
  });

  latch1.Wait();
  latch2.Wait();

  fml::TaskQueueId qid1 = loop1->GetTaskRunner()->GetTaskQueueId();
  fml::TaskQueueId qid2 = loop2->GetTaskRunner()->GetTaskQueueId();
  const auto task_runner_merger_ =
      fml::MakeRefCounted<fml::TaskRunnerMerger>(qid1, qid2);
  const int kNumFramesMerged = 5;

  ASSERT_FALSE(task_runner_merger_->AreMerged());

  task_runner_merger_->MergeWithLease(kNumFramesMerged);

  for (int i = 0; i < kNumFramesMerged; i++) {
    ASSERT_TRUE(task_runner_merger_->AreMerged());
    task_runner_merger_->DecrementLease();
  }

  ASSERT_FALSE(task_runner_merger_->AreMerged());

  term1.Signal();
  term2.Signal();
  thread1.join();
  thread2.join();
}

TEST(TaskRunnerMerger, OnWrongThread) {
  fml::MessageLoop* loop1 = nullptr;
  fml::AutoResetWaitableEvent latch1;
  std::thread thread1([&loop1, &latch1]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    loop1 = &fml::MessageLoop::GetCurrent();
    loop1->GetTaskRunner()->PostTask([&]() { latch1.Signal(); });
    loop1->Run();
  });

  fml::MessageLoop* loop2 = nullptr;
  fml::AutoResetWaitableEvent latch2;
  std::thread thread2([&loop2, &latch2]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    loop2 = &fml::MessageLoop::GetCurrent();
    loop2->GetTaskRunner()->PostTask([&]() { latch2.Signal(); });
    loop2->Run();
  });

  latch1.Wait();
  latch2.Wait();

  fml::TaskQueueId qid1 = loop1->GetTaskRunner()->GetTaskQueueId();
  fml::TaskQueueId qid2 = loop2->GetTaskRunner()->GetTaskQueueId();
  const auto task_runner_merger_ =
      fml::MakeRefCounted<fml::TaskRunnerMerger>(qid1, qid2);

  fml::CountDownLatch pre_merge(2), post_merge(2), post_unmerge(2);

  loop1->GetTaskRunner()->PostTask([&]() {
    ASSERT_TRUE(task_runner_merger_->OnWrongThread());
    ASSERT_EQ(fml::MessageLoop::GetCurrentTaskQueueId(), qid1);
    pre_merge.CountDown();
  });

  loop2->GetTaskRunner()->PostTask([&]() {
    ASSERT_FALSE(task_runner_merger_->OnWrongThread());
    ASSERT_EQ(fml::MessageLoop::GetCurrentTaskQueueId(), qid2);
    pre_merge.CountDown();
  });

  pre_merge.Wait();

  task_runner_merger_->MergeWithLease(1);

  loop1->GetTaskRunner()->PostTask([&]() {
    ASSERT_FALSE(task_runner_merger_->OnWrongThread());
    ASSERT_EQ(fml::MessageLoop::GetCurrentTaskQueueId(), qid1);
    post_merge.CountDown();
  });

  loop2->GetTaskRunner()->PostTask([&]() {
    // this will be false since this is going to be run
    // on loop1 really.
    ASSERT_FALSE(task_runner_merger_->OnWrongThread());
    ASSERT_EQ(fml::MessageLoop::GetCurrentTaskQueueId(), qid1);
    post_merge.CountDown();
  });

  post_merge.Wait();

  task_runner_merger_->DecrementLease();

  loop1->GetTaskRunner()->PostTask([&]() {
    ASSERT_TRUE(task_runner_merger_->OnWrongThread());
    ASSERT_EQ(fml::MessageLoop::GetCurrentTaskQueueId(), qid1);
    post_unmerge.CountDown();
  });

  loop2->GetTaskRunner()->PostTask([&]() {
    ASSERT_FALSE(task_runner_merger_->OnWrongThread());
    ASSERT_EQ(fml::MessageLoop::GetCurrentTaskQueueId(), qid2);
    post_unmerge.CountDown();
  });

  post_unmerge.Wait();

  loop1->GetTaskRunner()->PostTask([&]() { loop1->Terminate(); });

  loop2->GetTaskRunner()->PostTask([&]() { loop2->Terminate(); });

  thread1.join();
  thread2.join();
}
