//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SUBSCRIPTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SUBSCRIPTION_H

#include <functional>

namespace pf::events {
class Subscription {
 public:
  using Unsubscriber = std::function<void()>;
  explicit Subscription(Unsubscriber &&unsub);

  void unsubscribe();

 private:
  Unsubscriber unsub;
};
}// namespace pf::events

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SUBSCRIPTION_H
