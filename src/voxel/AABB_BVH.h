/**
 * @file AABB_BVH.h
 * @brief Structures and algorithms for BVH.
 * @author Petr Flajšingr
 * @date 3.6.21
 */
#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_AABB_BVH_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_AABB_BVH_H

#include <glm/gtx/extended_min_max.hpp>
#include <ostream>
#include <pf_common/Tree.h>
#include <pf_common/math/BoundingBox.h>
#include <range/v3/range/conversion.hpp>
#include <ranges>
#include <voxel/GPUModelInfo.h>

namespace pf::vox {

namespace details {
/**
 * @brief A structure which is the same as the data saved in the gpu.
 */
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
/**
 * @brief BVH data stored in cpu representation.
 */
struct BVHData {
  math::BoundingBox<3> aabb;
  std::uint32_t modelIndex;
  [[nodiscard]] details::GPUBVHNode toGPUData() const;
};

namespace details {
using Node = Node<BVHData>;
}

/**
 * @brief Info about created BVH including some optional stats.
 */
struct BVHCreateInfo {
  Tree<BVHData> data;
  std::size_t depth = 0;
  std::size_t nodeCount = 0;
};
/**
 * Find 2 closest nodes in the vector and create a parent node.
 * @param nodes input nodes
 * @return new parent node of 2 closest nodes
 */
std::unique_ptr<details::Node> createNodeFromClosest2(std::vector<std::unique_ptr<details::Node>> &nodes);
/**
 * Create a new tree level from nodes.
 * @param nodes nodes to create parents for
 * @return newly created nodes containing passed ones as children
 */
std::vector<std::unique_ptr<details::Node>> createNextLevel(std::vector<std::unique_ptr<details::Node>> &&nodes);
/**
 * Create a new AABB from the original one and a transform matrix.
 * @param original original AABB
 * @param matrix transform matrix
 * @return new AABB
 */
math::BoundingBox<3> aabbFromTransformed(const math::BoundingBox<3> &original, const glm::mat4 &matrix);
/**
 * Create a BVH for models.
 * @param models models to create BVH for.
 * @param createStats if true additional stats are computed for the BVH
 * @return newly created BVH
 */
BVHCreateInfo
createBVH(std::ranges::range auto &&models,
          bool createStats) requires(std::same_as<std::ranges::range_value_t<decltype(models)>, GPUModelInfo>) {
  if (std::ranges::empty(models)) { return BVHCreateInfo{{}, 0, 0}; }

  auto nodes = models | std::views::transform([](const auto &model) {
                 const auto transformedAABB = aabbFromTransformed(model.AABB, model.transformMatrix);
                 return std::make_unique<details::Node>(BVHData{transformedAABB, *model.getModelIndex()});
               })
      | ranges::to_vector;

  while (nodes.size() > 1) { nodes = createNextLevel(std::move(nodes)); }
  auto result = BVHCreateInfo{};
  result.data = Tree<BVHData>{std::move(nodes.back())};
  if (createStats) {
    std::ranges::for_each(result.data.iterDepthFirst(), [&result](auto) { ++result.nodeCount; });
    auto totalModelCount = std::ranges::size(models);
    result.depth = 1;
    while ((totalModelCount = totalModelCount / 2 + totalModelCount % 2) > 2) { ++result.depth; }
    result.depth += totalModelCount;
  }
  return result;
}

namespace details {
  void serializeBVHForGPU(const details::Node &root, std::vector<details::GPUBVHNode> &result);
}// namespace details
/**
 * Copy BVH tree into GPU memory.
 * @param bvh source data
 * @param mapping destination
 */
void saveBVHToBuffer(const Tree<BVHData> &bvh, vulkan::BufferMapping &mapping);

}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_AABB_BVH_H
