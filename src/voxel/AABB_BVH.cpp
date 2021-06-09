//
// Created by petr on 6/4/21.
//

#include "AABB_BVH.h"
#include <fmt/ostream.h>
#include <logging/loggers.h>
#include <pf_common/concepts/StringConvertible.h>
#include <pf_common/views/View2D.h>

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
// TODO: implement on gpu
std::unique_ptr<Node<BVHData>> createNodeFromClosest2(std::vector<std::unique_ptr<Node<BVHData>>> &nodes) {
  auto distanceMatrix = std::vector<float>(std::pow(nodes.size(), 2));
  auto distance2DView = makeView2D(distanceMatrix, nodes.size());
  for (std::size_t diag = 0; diag < nodes.size(); ++diag) {
    distance2DView[diag][diag] = std::numeric_limits<float>::max();
  }
  std::ranges::for_each(nodes | ranges::views::enumerate, [&nodes, &distance2DView](const auto &nodeIdx1) {
    const auto &[idx1, node1] = nodeIdx1;
    std::ranges::for_each(nodes | ranges::views::enumerate, [idx1, &node1, &distance2DView](const auto &nodeIdx2) {
      const auto &[idx2, node2] = nodeIdx2;
      if (idx1 == idx2) { return; }
      distance2DView[idx1][idx2] = node1->value().aabb.distance(node2->value().aabb);
    });
  });
  auto smallestIdx = 0;
  auto smallestDistance = std::numeric_limits<float>::max();
  for (const auto &[idx, distance] : distanceMatrix | ranges::views::enumerate) {
    if (distance < smallestDistance) {
      smallestIdx = idx;
      smallestDistance = distance;
    }
  }
  const auto indexOfFirst = smallestIdx % nodes.size();
  const auto indexOfSecond = smallestIdx / nodes.size();

  auto &first = nodes[indexOfFirst];
  auto &second = nodes[indexOfSecond];
  auto newNode = std::make_unique<details::Node>(BVHData{first->value().aabb.combine(second->value().aabb), 0});
  newNode->appendChild(std::move(first));
  newNode->appendChild(std::move(second));
  nodes.erase(nodes.begin() + indexOfFirst);
  if (indexOfFirst < indexOfSecond) {
    nodes.erase(nodes.begin() + indexOfSecond - 1);
  } else {
    nodes.erase(nodes.begin() + indexOfSecond);
  }
  return newNode;
}

std::vector<std::unique_ptr<Node<BVHData>>> createNextLevel(std::vector<std::unique_ptr<Node<BVHData>>> &&nodes) {
  auto result = std::vector<std::unique_ptr<details::Node>>{};

  while (nodes.size() > 1) { result.emplace_back(createNodeFromClosest2(nodes)); }
  if (nodes.size() == 1) {
    result.emplace_back(std::move(nodes.back()));
    nodes.pop_back();
  }

  return result;
}
void saveBVHToBuffer(const Tree<BVHData> &bvh, vulkan::BufferMapping &mapping) {
  if (!bvh.hasRoot()) { return; }
  auto gpuNodes = std::vector<details::GPUBVHNode>{};

  auto &root = bvh.getRoot();

  auto &rootData = gpuNodes.emplace_back(root->toGPUData());
  const auto isRootLeaf = root.childrenSize() == 0;
  rootData.setIsLeaf(isRootLeaf);
  if (isRootLeaf) {
    rootData.setOffset(root->modelIndex);
  } else {
    rootData.setOffset(gpuNodes.size());
  }
  details::serializeBVHForGPU(root, gpuNodes);
  mapping.set(gpuNodes);
}

void details::serializeBVHForGPU(const Node &root, std::vector<details::GPUBVHNode> &result) {
  if (root.childrenSize() == 0) { return; }
  auto &child1 = root.children()[0];
  const auto isChild1Leaf = child1.childrenSize() == 0;
  result.emplace_back(child1->toGPUData());
  auto nodeChild1Idx = result.size() - 1;
  result[nodeChild1Idx].setIsLeaf(isChild1Leaf);
  auto &child2 = root.children()[1];
  const auto isChild2Leaf = child2.childrenSize() == 0;
  result.emplace_back(child2->toGPUData());
  auto nodeChild2Idx = result.size() - 1;
  result[nodeChild2Idx].setIsLeaf(isChild2Leaf);
  if (isChild1Leaf) {
    result[nodeChild1Idx].setOffset(child1->modelIndex);
  } else {
    result[nodeChild1Idx].setOffset(result.size());
    serializeBVHForGPU(child1, result);
  }
  if (isChild2Leaf) {
    result[nodeChild2Idx].setOffset(child2->modelIndex);
  } else {
    result[nodeChild2Idx].setOffset(result.size());
    serializeBVHForGPU(child2, result);
  }
}
}// namespace pf::vox