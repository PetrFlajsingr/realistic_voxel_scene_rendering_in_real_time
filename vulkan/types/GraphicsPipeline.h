//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GRAPHICSPIPELINE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GRAPHICSPIPELINE_H

#include "../concepts/PtrConstructible.h"
#include "VulkanObject.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

class GraphicsPipeline : public VulkanObject, public PtrConstructible<GraphicsPipeline> {
 public:
  // TODO: builder only
  explicit GraphicsPipeline(vk::UniquePipeline &&pipeline);

  GraphicsPipeline(const GraphicsPipeline &other) = delete;
  GraphicsPipeline &operator=(const GraphicsPipeline &other) = delete;

  [[nodiscard]] const vk::Pipeline &getVkPipeline() const;

  const vk::Pipeline &operator*() const;
  vk::Pipeline const *operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  vk::UniquePipeline vkPipeline;
};

}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GRAPHICSPIPELINE_H
