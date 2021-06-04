//
// Created by petr on 6/4/21.
//

#include "AABB_BVH.h"
#include <fmt/ostream.h>
#include <logging/loggers.h>
#include <pf_common/concepts/StringConvertible.h>

namespace pf::vox {

details::GPUBVHNode &details::GPUBVHNode::setIsLeaf(bool isLeaf) {
  if (isLeaf) {
    aabb2leafNext.z = std::bit_cast<float>(std::bit_cast<std::uint32_t>(aabb2leafNext.z) | LEAF_NODE_MASK);
  } else {
    aabb2leafNext.z = std::bit_cast<float>(std::bit_cast<std::uint32_t>(aabb2leafNext.z) & ~LEAF_NODE_MASK);
  }
  return *this;
}

details::GPUBVHNode &details::GPUBVHNode::setOffset(std::uint32_t offset) {
  aabb2leafNext.z =
      std::bit_cast<float>((std::bit_cast<std::uint32_t>(aabb2leafNext.z) & ~OFFSET_MASK) | (offset & OFFSET_MASK));

  return *this;
}

std::uint32_t details::GPUBVHNode::getOffset() const {
  return std::bit_cast<std::uint32_t>(aabb2leafNext.z) & OFFSET_MASK;
}

bool details::GPUBVHNode::isLeaf() const { return std::bit_cast<std::uint32_t>(aabb2leafNext.z) & LEAF_NODE_MASK; }

math::BoundingBox<3> details::GPUBVHNode::getAABB() const {
  return math::BoundingBox<3>{{aabb1.xyz()}, {aabb1.w, aabb2leafNext.xy()}};
}
std::ostream &details::operator<<(std::ostream &os, const details::GPUBVHNode &node) {
  os << "AABB: " << node.getAABB() << " offset: " << node.getOffset() << " is leaf: " << std::boolalpha
     << node.isLeaf();
  return os;
}

details::GPUBVHNode BVHData::toGPUData() const {
  details::GPUBVHNode result;
  result.aabb1 = glm::vec4{aabb.p1, aabb.p2.x};
  result.aabb2leafNext = glm::vec4{aabb.p2.yz(), 0, 0};
  return result;
}

std::unique_ptr<Node<BVHData>> createNodeFrom2(std::vector<std::unique_ptr<Node<BVHData>>> &nodes) {
  auto closestDist = std::numeric_limits<float>::max();
  auto closestIdx = 0;

  auto current = std::move(nodes.back());
  nodes.pop_back();
  for (const auto &[idx, node] : nodes | ranges::views::enumerate) {
    const auto distance = node->value().aabb.distance(current->value().aabb);
    if (distance < closestDist) {
      closestDist = distance;
      closestIdx = idx;
    }
  }
  auto &closest = nodes[closestIdx];
  auto newNode = std::make_unique<details::Node>(BVHData{current->value().aabb.combine(closest->value().aabb), 0});
  newNode->appendChild(std::move(current));
  newNode->appendChild(std::move(closest));
  nodes.erase(nodes.begin() + closestIdx);
  return newNode;
}

std::vector<std::unique_ptr<Node<BVHData>>> createNextLevel(std::vector<std::unique_ptr<Node<BVHData>>> &&nodes) {
  auto result = std::vector<std::unique_ptr<details::Node>>{};

  while (nodes.size() > 1) { result.emplace_back(createNodeFrom2(nodes)); }
  if (nodes.size() == 1) {
    result.emplace_back(std::move(nodes.back()));
    nodes.pop_back();
  }

  return result;
}

void saveBVHToBuffer(const Tree<BVHData> &bvh, vulkan::BufferMapping &mapping) {
  if (!bvh.hasRoot()) { return; }
  auto gpuNodes = std::vector<std::unique_ptr<details::GPUBVHNode>>{};

  auto &root = bvh.getRoot();

  auto &rootData = gpuNodes.emplace_back(std::make_unique<details::GPUBVHNode>(root->toGPUData()));
  const auto isRootLeaf = root.childrenSize() == 0;
  rootData->setIsLeaf(isRootLeaf);
  if (isRootLeaf) {
    rootData->setOffset(root->modelIndex);
  } else {
    rootData->setOffset(gpuNodes.size());
  }
  details::serializeBVHForGPU(root, gpuNodes);
  for (auto &node : gpuNodes) { logd("BVH", "{}", *node); }
  mapping.set(gpuNodes | std::views::transform([](const auto &dataPtr) { return *dataPtr; }) | ranges::to_vector);
}

void details::serializeBVHForGPU(const Node &root, std::vector<std::unique_ptr<details::GPUBVHNode>> &result) {
  if (root.childrenSize() == 0) { return; }
  auto &child1 = root.children()[0];
  const auto isChild1Leaf = child1.childrenSize() == 0;
  auto nodeChild1 = result.emplace_back(std::make_unique<details::GPUBVHNode>()).get();
  *nodeChild1 = child1->toGPUData();
  nodeChild1->setIsLeaf(isChild1Leaf);
  auto &child2 = root.children()[1];
  const auto isChild2Leaf = child2.childrenSize() == 0;
  auto nodeChild2 = result.emplace_back(std::make_unique<details::GPUBVHNode>()).get();
  *nodeChild2 = child2->toGPUData();
  nodeChild2->setIsLeaf(isChild2Leaf);
  if (isChild1Leaf) {
    nodeChild1->setOffset(child1->modelIndex);
  } else {
    nodeChild1->setOffset(result.size());
    serializeBVHForGPU(child1, result);
  }
  if (isChild2Leaf) {
    nodeChild2->setOffset(child2->modelIndex);
  } else {
    nodeChild2->setOffset(result.size());
    serializeBVHForGPU(child2, result);
  }
}
}// namespace pf::vox