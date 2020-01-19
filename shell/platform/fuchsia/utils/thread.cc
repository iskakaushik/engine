// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/utils/thread.h"

#include <condition_variable>
#include <mutex>
#include <utility>

#include <lib/async-loop/default.h>
#include <lib/async/cpp/task.h>
#include <lib/async/default.h>
#include <lib/zx/process.h>
#include <lib/zx/thread.h>

#include "flutter/shell/platform/fuchsia/utils/logging.h"

namespace fx {

void Thread::SetProcessName(std::string process_name) {
  zx::process::self()->set_property(ZX_PROP_NAME, process_name.c_str(),
                                    process_name.size());
}

void Thread::SetCurrentThreadName(std::string thread_name) {
  zx::thread::self()->set_property(ZX_PROP_NAME, thread_name.c_str(),
                                   thread_name.size());
}

Thread::Thread(std::string name)
    : loop_(std::make_unique<async::Loop>(
          &kAsyncLoopConfigNoAttachToCurrentThread)),
      thread_([this, thread_name = std::move(name)]() {
        Thread::SetCurrentThreadName(thread_name);
        async_set_default_dispatcher(loop_->dispatcher());
        loop_->Run();
      }) {}

Thread::~Thread() {
  Join();
}

async_dispatcher_t* Thread::dispatcher() const {
  return loop_->dispatcher();
}

void Thread::TaskBarrier(std::function<void()> task) const {
  std::pair<std::mutex, std::condition_variable> latch;
  bool signal = false;

  // Run the task on this Thread's dispatcher and signal the latch.
  async::PostTask(dispatcher(), [&]() {
    task();

    std::lock_guard<std::mutex> locker(latch.first);
    signal = true;
    latch.second.notify_one();
  });

  // Make the task synchronous by waiting on the latch.
  std::unique_lock<std::mutex> locker(latch.first);
  latch.second.wait(locker, [&signal]() -> bool { return signal; });
}

void Thread::Join() {
  if (joined_ || !loop_ || !thread_.joinable()) {
    return;
  }
  joined_ = true;

  async::PostTask(loop_->dispatcher(), [this]() { loop_->Quit(); });
  thread_.join();
}

}  // namespace fx
