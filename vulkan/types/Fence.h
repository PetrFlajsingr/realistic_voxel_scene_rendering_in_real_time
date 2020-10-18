//
// Created by petr on 10/18/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FENCE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FENCE_H

#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct FenceConfig {
  uint32_t semaphoreCount;
};

class Fence {};

}

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FENCE_H
