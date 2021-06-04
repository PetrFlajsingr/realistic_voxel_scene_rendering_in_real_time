//
// Created by petr on 6/3/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_AABB_BVH_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_AABB_BVH_H

#include <ostream>
#include <pf_common/Tree.h>
#include <pf_common/math/BoundingBox.h>
#include <ranges>
#include <voxel/GPUModelInfo.h>

namespace pf::vox {

namespace details {
struct alignas(16) GPUBVHNode {
  glm::vec4 aabb1;
  glm::vec4 aabb2leafNext;
  GPUBVHNode &setIsLeaf(bool isLeaf);
  GPUBVHNode &setOffset(std::uint32_t offset);
  [[nodiscard]] std::uint32_t getOffset() const;
  [[nodiscard]] bool isLeaf() const;
  [[nodiscard]] math::BoundingBox<3> getAABB() const;

  constexpr static std::uint32_t OFFSET_MASK = 0b01111111111111111111111111111111;
  constexpr static std::uint32_t LEAF_NODE_MASK = ~OFFSET_MASK;
};
std::ostream &operator<<(std::ostream &os, const GPUBVHNode &node);
}// namespace details
struct BVHData {
  math::BoundingBox<3> aabb;
  std::uint32_t modelIndex;
  details::GPUBVHNode toGPUData() const;
};

namespace details {
using Node = Node<BVHData>;
}

std::unique_ptr<details::Node> createNodeFrom2(std::vector<std::unique_ptr<details::Node>> &nodes);

std::vector<std::unique_ptr<details::Node>> createNextLevel(std::vector<std::unique_ptr<details::Node>> &&nodes);

Tree<BVHData> createBVH(std::ranges::range auto &&models) requires(
    std::same_as<std::ranges::range_value_t<decltype(models)>, GPUModelInfo>) {
  if (std::ranges::empty(models)) { return Tree<BVHData>{}; }

  auto nodes =
      models | std::views::transform([](const auto &model) {
        return std::make_unique<details::Node>(BVHData{model.transformMatrix * model.AABB, *model.getModelIndex()});
      })
      | ranges::to_vector;

  while (nodes.size() > 1) { nodes = createNextLevel(std::move(nodes)); }
  return Tree<BVHData>{std::move(nodes.back())};
}

namespace details {
  void serializeBVHForGPU(const details::Node &root, std::vector<std::unique_ptr<details::GPUBVHNode>> &result);
}// namespace details

void saveBVHToBuffer(const Tree<BVHData> &bvh, vulkan::BufferMapping &mapping);

}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_AABB_BVH_H
