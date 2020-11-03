//
// Created by petr on 9/24/20.
//

#ifndef VOXEL_RENDER_SUBSCRIPTION_H
#define VOXEL_RENDER_SUBSCRIPTION_H

#include <functional>

namespace pf::events {
class Subscription {
 public:
  using Unsubscriber = std::function<void()>;
  explicit Subscription(Unsubscriber &&unsubscriber);
  Subscription(const Subscription &) = delete;
  Subscription &operator=(const Subscription &) = delete;
  Subscription(Subscription &&other)  noexcept;
  Subscription &operator=(Subscription &&other)  noexcept;

  void unsubscribe();

 private:
  Unsubscriber unsub;
};
}// namespace pf::events

#endif//VOXEL_RENDER_SUBSCRIPTION_H
