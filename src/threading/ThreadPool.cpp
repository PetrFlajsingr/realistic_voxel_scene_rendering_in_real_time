//
// Created by petr on 10/27/20.
//

#include "ThreadPool.h"
#include <iostream>
#include <range/v3/view.hpp>

pf::ThreadPool::ThreadPool(std::size_t poolSize) {
  for (std::size_t i = 0; i < poolSize; ++i) { threads.emplace_back(getThreadFunction()); }
}

pf::ThreadPool::~ThreadPool() {
  if (state == ThreadPoolState::Run) { state = ThreadPoolState::FinishAndStop; }
  poolCv.notify_all();
  std::ranges::for_each(threads, [](auto &thread) { thread.join(); });
}

void pf::ThreadPool::finishAndStop() { state = ThreadPoolState::FinishAndStop; }
void pf::ThreadPool::cancelAndStop() { state = ThreadPoolState::Stop; }

std::function<void()> pf::ThreadPool::getThreadFunction() {
  return [&] {
    auto lock = std::unique_lock(queueMtx, std::defer_lock);
    while (true) {
      lock.lock();
      poolCv.wait(lock, [&] { return !taskQueue.empty() || state != ThreadPoolState::Run; });
      switch (state) {
        case ThreadPoolState::Stop: return;
        case ThreadPoolState::FinishAndStop:
          if (taskQueue.empty()) { return; }
          break;
        case ThreadPoolState::Run: break;
      }
      auto task = std::move(taskQueue.front());
      taskQueue.pop();
      lock.unlock();
      (*task)();
    }
  };
}
