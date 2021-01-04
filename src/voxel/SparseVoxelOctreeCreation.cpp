//
// Created by petr on 12/15/20.
//

#include "SparseVoxelOctreeCreation.h"
#include "SparseVoxelOctree.h"
#include <magic_enum.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
#include <utils/bits.h>

namespace pf::vox {
using namespace ranges;

SparseVoxelOctree loadFileAsSVO(const std::filesystem::path &srcFile, FileType fileType) {
  if (fileType == FileType::Unknown) {
    const auto detectedFileType = details::detectFileType(srcFile);
    if (!detectedFileType.has_value()) { throw LoadException("Could not detect file type for '{}'", srcFile.string()); }
    fileType = *detectedFileType;
  }
  auto ifstream = std::ifstream(srcFile, std::ios::binary);
  if (!ifstream.is_open()) { throw LoadException("Could not open file '{}'", srcFile.string()); }
  switch (fileType) {
    case FileType::Vox: return details::loadVoxFileAsSVO(std::move(ifstream)); break;
    default:
      throw LoadException("Could not load model '{}', unsupported format: {}", srcFile.string(),
                          magic_enum::enum_name(fileType));
  }
}

namespace details {
SparseVoxelOctree loadVoxFileAsSVO(std::ifstream &&istream) {

  const auto scene = loadVoxScene(std::move(istream));

  const auto bb = findBB(scene);
  const auto octreeLevels = calcOctreeLevelCount(bb);

  auto voxels = scene.getModels() | views::transform([](const auto &model) { return model->getVoxels() | views::all; })
      | views::join | to_vector | actions::sort([](const auto &a, const auto &b) {
                  return a.position.x < b.position.x || a.position.y < b.position.y || a.position.z < b.position.z;
                });

  auto tree = Tree<TemporaryTreeNode, 8>::BuildTree(octreeLevels, TemporaryTreeNode{});

  std::ranges::for_each(
      voxels, [&tree, octreeLevels](const auto &voxel) { details::addVoxelToTree(tree, voxel, octreeLevels); });

  return rawTreeToSVO(tree);
}

math::BoundingBox<3> findBB(const Scene &scene) {
  constexpr auto MAX = std::numeric_limits<float>::max();
  constexpr auto MIN = std::numeric_limits<float>::lowest();

  auto result = math::BoundingBox<3>(glm::vec3{MAX}, glm::vec3{MIN});

  for (const auto &model : scene.getModels()) {
    for (const auto &voxel : model->getVoxels()) {
      result.p1.x = std::min(voxel.position.x, result.p1.x);
      result.p1.y = std::min(voxel.position.y, result.p1.y);
      result.p1.z = std::min(voxel.position.z, result.p1.z);

      result.p2.x = std::max(voxel.position.x + 1, result.p2.x);
      result.p2.y = std::max(voxel.position.y + 1, result.p2.y);
      result.p2.z = std::max(voxel.position.z + 1, result.p2.z);
    }
  }
  return result;
}

uint32_t calcOctreeLevelCount(const math::BoundingBox<3> &bb) {
  const auto xSize = bb.p2.x - bb.p1.x;
  const auto ySize = bb.p2.y - bb.p1.y;
  const auto zSize = bb.p2.z - bb.p1.z;

  const auto length = std::max({xSize, ySize, zSize});
  for (uint32_t i = 0; i < OCTREE_DEPTH_LIMIT; ++i) {
    const auto levelLength = std::pow(2, i);
    if (length <= levelLength) { return i; }
  }
  throw LoadException::fmt("Octree depth higher than limit '{}'", OCTREE_DEPTH_LIMIT);
}

math::BoundingBox<3> bbToOctreeBB(math::BoundingBox<3> bb, uint32_t levels) {
  const auto alignedLength = std::pow(2, levels);
  const auto xDiff = bb.p2.x - bb.p1.x;
  const auto yDiff = bb.p2.y - bb.p1.y;
  const auto zDiff = bb.p2.z - bb.p1.z;

  if (xDiff < alignedLength) { bb.p2.x += alignedLength - xDiff; }
  if (yDiff < alignedLength) { bb.p2.y += alignedLength - yDiff; }
  if (zDiff < alignedLength) { bb.p2.z += alignedLength - zDiff; }
  return bb;
}

uint32_t idxForLevel(glm::vec3 pos, uint32_t level, uint32_t depth) {
  const auto partLength = std::floor(std::pow(2, depth) / (level + 1) / 2);
  if (level > 0) {
    const auto parentPartLength = std::floor(std::pow(2, depth) / level / 2);
    pos = glm::ivec3(pos) % static_cast<int>(parentPartLength);
  }
  const auto vecIndices = glm::floor(pos / static_cast<float>(partLength));
  return vecIndices.x + 2 * vecIndices.y + 4 * vecIndices.z;
}

void addVoxelToTree(Tree<TemporaryTreeNode, 8> &tree, const Voxel &voxel, uint32_t octreeLevels) {
  auto node = &tree.getRoot();
  (*node)->isValid = true;
  for (uint32_t i = 0; i < octreeLevels; ++i) {
    const auto idx = idxForLevel(voxel.position.xyz(), i, octreeLevels);
    auto tmpNode = &node->childAtIndex(idx);
    (*tmpNode)->isValid = true;
    (*tmpNode)->debug.position = fmt::format("{}x{}x{}", voxel.position.x, voxel.position.y, voxel.position.z);
    if (tmpNode->getType() == NodeType::Node) { node = &node->childAtIndex(idx).asNode(); }
  }
}

ChildDescriptor childDescriptorForNode(const Node<TemporaryTreeNode, 8> &node) {
  auto result = ChildDescriptor();
  result.childData.far = 0;
  result.childData.leafMask = 0;
  for (uint32_t i = 0; i < 8; ++i) {
    if (node.childAtIndex(i)->isValid) {
      result.childData.validMask |= 1 << i;
    } else {
      result.childData.validMask &= ~(1 << i);
    }
    if (node.childAtIndex(i).getType() == NodeType::Leaf) { result.childData.leafMask |= 1 << i; }
  }
  return result;
}

std::size_t countNonLeafChildrenForNode(const Leaf<TemporaryTreeNode, 8> &leaf) {
  return std::ranges::count_if(leaf.asNode().getChildren(), [](const auto &child) {
    return child->getType() == NodeType::Node && (*child)->isValid;
  });
}

auto getValidChildren(const Node<TemporaryTreeNode, 8> &node) {
  return node.getChildren()
      | views::filter([](const auto &child) { return child->getType() == NodeType::Node && (*child)->isValid; })
      | views::transform([](const auto &child) { return &child->asNode(); });
}

auto getValidChildren(const std::vector<Node<TemporaryTreeNode, 8> *> &nodes) {
  return nodes | views::transform([](const auto &node) { return getValidChildren(*node); }) | views::join;
}

std::vector<ChildDescriptor> buildDescriptors(const std::vector<Node<TemporaryTreeNode, 8> *> &nodes,
                                              uint32_t &childPointer) {
  auto result = std::vector<ChildDescriptor>();

  for (const auto node : nodes) {
    const auto descriptor = childDescriptorForNode(*node);
    result.emplace_back(descriptor);
  }

  auto nodesWithDescriptors = views::zip(nodes, result);

  auto begin = nodesWithDescriptors.begin();
  const auto end = nodesWithDescriptors.end();
  if (begin != end) {
    {
      childPointer += std::distance(begin, end);
      const auto &[child, descriptor] = *begin;
      descriptor.childData.childPointer = childPointer;
      ++begin;
    }
    for (; begin != end; ++begin) {
      const auto &[child, descriptor] = *begin;
      childPointer += countNonLeafChildrenForNode(*child);
      descriptor.childData.childPointer = childPointer;
    }
  }
  auto validChildren = getValidChildren(nodes) | to_vector;
  if (!validChildren.empty()) {
    const auto descriptors = buildDescriptors(validChildren, childPointer);
    const auto originalResultSize = result.size();
    result.resize(originalResultSize + descriptors.size());
    std::ranges::copy(descriptors, result.begin() + originalResultSize);
  }
  return result;
}

SparseVoxelOctree rawTreeToSVO(const Tree<TemporaryTreeNode, 8> &tree) {
  // TODO:
  auto childDescriptors = std::vector<ChildDescriptor>();

  const auto &root = tree.getRoot();

  auto rootDescriptor = childDescriptorForNode(root);
  auto childPointer = 1u;
  rootDescriptor.childData.childPointer = childPointer;

  childDescriptors.emplace_back(rootDescriptor);

  if (root.getType() == NodeType::Node) {
    const auto validChildren = getValidChildren(root) | to_vector;
    const auto descriptors = buildDescriptors(validChildren, childPointer);
    childDescriptors.resize(childDescriptors.size() + descriptors.size());
    std::ranges::copy(descriptors, childDescriptors.begin() + 1);
  }

  auto page = Page();
  page.childDescriptors = std::move(childDescriptors);
  page.header.infoSectionPointer = 0;
  page.farPointers = {};

  auto block = Block();
  block.pages = {std::move(page)};
  block.contourData = {};
  block.infoSection = {};

  return SparseVoxelOctree({std::move(block)});
}

}// namespace details

}// namespace pf::vox