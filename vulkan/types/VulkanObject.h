//
// Created by petr on 10/12/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKANOBJECT_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKANOBJECT_H

#include <memory>
#include <string>

namespace pf::vulkan {
class VulkanObject {
 public:
  [[nodiscard]] virtual std::string info() const = 0;
  virtual ~VulkanObject() = default;

  inline friend std::ostream &operator<<(std::ostream &ostream, const VulkanObject &obj) {
    return ostream << obj.info();
  }
};
}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKANOBJECT_H
