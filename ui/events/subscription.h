//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SUBSCRIPTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SUBSCRIPTION_H

#include <functional>

namespace pf::events {
class subscription {
 public:
  using unsubscriber = std::function<void()>;
  explicit subscription(unsubscriber &&unsub);

  void unsubscribe();
 private:
  unsubscriber unsub;
};
}

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SUBSCRIPTION_H
