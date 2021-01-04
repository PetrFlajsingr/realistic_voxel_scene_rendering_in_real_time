//
// Created by petr on 10/27/20.
//

#ifndef VOXEL_RENDER_THREADPOOL_H
#define VOXEL_RENDER_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <thread>

namespace pf {

namespace details {
struct Task {
  virtual void operator()() = 0;
  virtual ~Task() = default;
};
template<std::invocable F>
class ConcreteTask : public Task {
 public:
  explicit ConcreteTask(F &&callable) : fnc(std::forward<F>(callable)) {}
  void operator()() override { fnc(); }

 private:
  F fnc;
};
}// namespace details

enum class ThreadPoolState { Run, Stop, FinishAndStop };

class ThreadPool {
 public:
  explicit ThreadPool(std::size_t poolSize);
  ~ThreadPool();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;

  template<typename... Args>
  auto enqueue(std::invocable<Args...> auto &&callable, Args &&...args) {
    using ResultValueType = std::invoke_result_t<decltype(callable), Args...>;
    using TaskType = std::packaged_task<ResultValueType()>;
    auto task = TaskType{std::bind(callable, std::forward<Args>(args)...)};
    auto future = task.get_future();
    {
      auto lock = std::unique_lock{queueMtx};
      taskQueue.emplace(std::make_unique<details::ConcreteTask<TaskType>>(std::move(task)));
    }
    poolCv.notify_one();
    return future;
  }

  void finishAndStop();
  void cancelAndStop();

 private:
  std::function<void()> getThreadFunction();
  std::vector<std::thread> threads;
  ThreadPoolState state = ThreadPoolState::Run;
  std::condition_variable poolCv;
  std::queue<std::unique_ptr<details::Task>> taskQueue;
  std::mutex queueMtx;
};

}// namespace pf
#endif//VOXEL_RENDER_THREADPOOL_H
