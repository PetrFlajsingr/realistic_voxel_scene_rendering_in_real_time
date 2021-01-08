//
// Created by petr on 12/15/20.
//

#include "SparseVoxelOctreeCreation.h"
#include "../../../pf_common/include/pf_common/bits.h"
#include "SparseVoxelOctree.h"
#include <magic_enum.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#define MINIMISE_TREE 1

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
SparseVoxelOctree convertSceneToSVO(const Scene &scene) {
  const auto bb = details::findBB(scene);
  [[maybe_unused]] const auto octreeLevels = details::calcOctreeLevelCount(bb);

  auto voxels = scene.getModels() | views::transform([](const auto &model) { return model->getVoxels() | views::all; })
      | views::join | to_vector | actions::sort([](const auto &a, const auto &b) {
                  return a.position.x < b.position.x && a.position.y < b.position.y && a.position.z < b.position.z;
                });

  auto tree = Tree<details::TemporaryTreeNode>();

  std::ranges::for_each(
      voxels, [&tree, octreeLevels](const auto &voxel) { details::addVoxelToTree(tree, voxel, octreeLevels); });

  return rawTreeToSVO(tree);
}

namespace details {
SparseVoxelOctree loadVoxFileAsSVO(std::ifstream &&istream) {

  const auto scene = loadVoxScene(std::move(istream));

  return convertSceneToSVO(scene);
}

math::BoundingBox<3> findBB(const Scene &scene) {
  //constexpr auto MAX = std::numeric_limits<float>::max();
  constexpr auto MIN = std::numeric_limits<float>::lowest();

  auto result = math::BoundingBox<3>(glm::vec3{0}, glm::vec3{MIN});

  auto voxels = scene.getModels() | views::transform([](const auto &model) { return model->getVoxels() | views::all; })
      | views::join;
  std::ranges::for_each(voxels, ([&result](const auto &voxel) {
                          result.p1.x = std::min(voxel.position.x, result.p1.x);
                          result.p1.y = std::min(voxel.position.y, result.p1.y);
                          result.p1.z = std::min(voxel.position.z, result.p1.z);

                          result.p2.x = std::max(voxel.position.x + 1, result.p2.x);
                          result.p2.y = std::max(voxel.position.y + 1, result.p2.y);
                          result.p2.z = std::max(voxel.position.z + 1, result.p2.z);
                        }));

  return result;
}

uint32_t calcOctreeLevelCount(const math::BoundingBox<3> &bb) {
  const auto xSize = bb.p2.x - bb.p1.x;
  const auto ySize = bb.p2.y - bb.p1.y;
  const auto zSize = bb.p2.z - bb.p1.z;

  const auto length = std::max({xSize, ySize, zSize});
  for (uint32_t i = 1; i < OCTREE_DEPTH_LIMIT; ++i) {
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
  const auto totalLength = std::pow(2, depth);
  const auto partLength = totalLength / std::pow(2, level) / 2;
  if (level > 0) {
    const auto parentPartLength = partLength * 2;
    pos = glm::ivec3(pos) % static_cast<int>(parentPartLength);
  }
  const auto vecIndices = glm::floor(pos / static_cast<float>(partLength));
  return vecIndices.x + 2 * vecIndices.y + 4 * vecIndices.z;
}

void addVoxelToTree(Tree<TemporaryTreeNode> &tree, const Voxel &voxel, uint32_t octreeLevels) {
  if (!tree.hasRoot()) { tree.initRoot(TemporaryTreeNode{}); }
  auto node = &tree.getRoot();
  (*node)->idx = 0;
  (*node)->isLeaf = false;
  std::string a = fmt::format("{}x{}x{}:", voxel.position.x, voxel.position.y, voxel.position.z);
  for (uint32_t i = 0; i < octreeLevels; ++i) {
    const auto idx = idxForLevel(voxel.position.xyz(), i, octreeLevels);
    a += std::to_string(idx) + " ";
    auto children = node->children();
    auto childNodeIter = std::ranges::find_if(children, [idx](const auto &child) { return child->idx == idx; });
    Node<TemporaryTreeNode> *childNode;
    if (childNodeIter == children.end()) {
      childNode = &node->appendChild();
      (*childNode)->idx = idx;
    } else {
      childNode = &*childNodeIter;
    }
    (*childNode)->debug.position = fmt::format("{}x{}x{}", voxel.position.x, voxel.position.y, voxel.position.z);
    node = childNode;
  }
  (*node)->isLeaf = true;
  std::cout << a << std::endl;
}

ChildDescriptor childDescriptorForNode(const Node<TemporaryTreeNode> &node) {
  auto result = ChildDescriptor();
  result.childData.far = 0;
  result.childData.leafMask = 0;
  result.childData.validMask = 0;
  for (const auto &child : node.children()) {
    result.childData.validMask |= 1u << child->idx;
    if (child->isLeaf) { result.childData.leafMask |= 1u << child->idx; }
  }
  return result;
}

std::size_t countNonLeafChildrenForNode(const Node<TemporaryTreeNode> &node) {
  return std::ranges::count_if(node.children(), [](const auto &child) { return !child->isLeaf; });
}

auto getValidChildren(const Node<TemporaryTreeNode> &node) {
  return node.children() | views::filter([](const auto &child) { return !child->isLeaf; })
      | views::transform([](const auto &child) { return &child; });
}

auto getValidChildren(const std::vector<const Node<TemporaryTreeNode> *> &nodes) {
  return nodes | views::transform([](const auto &node) { return getValidChildren(*node); }) | views::join;
}

std::vector<ChildDescriptor> buildDescriptors(const std::vector<const Node<TemporaryTreeNode> *> &nodes,
                                              uint32_t &childPointer) {
  auto result = nodes | views::transform([](const auto &node) { return childDescriptorForNode(*node); }) | to_vector;

  auto nodesWithDescriptors = views::zip(nodes, result);

  auto iter = nodesWithDescriptors.begin();
  const auto end = nodesWithDescriptors.end();
  auto tmpChildPointer = uint32_t();
  if (iter != end) {
    childPointer += std::distance(iter, end);
    const auto &[child, descriptor] = *iter;
    descriptor.childData.childPointer = childPointer;
    ++iter;
    tmpChildPointer = childPointer;
    for (; iter != end; ++iter) {
      const auto &[previousChild, _] = *(iter - 1);
      const auto &[child, descriptor] = *iter;
      childPointer += countNonLeafChildrenForNode(*previousChild);
      descriptor.childData.childPointer = childPointer;
    }
  }
  auto validChildren = getValidChildren(nodes) | to_vector;
  if (!validChildren.empty()) {
    childPointer = tmpChildPointer;
    const auto descriptors = buildDescriptors(validChildren, childPointer);
    const auto originalResultSize = result.size();
    result.resize(originalResultSize + descriptors.size());
    std::ranges::copy(descriptors, result.begin() + originalResultSize);
  }
  return result;
}

bool isNodeFilled(const Node<TemporaryTreeNode> &node) {
  if (node->isLeaf) {
    return true;
  }
  return node.childrenSize() == 8 && std::ranges::all_of(node.children(), isNodeFilled);
}

void setFilledNodesToLeaf(Node<TemporaryTreeNode> &node) {
  if (node->isLeaf) {
    return;
  }
  if (isNodeFilled(node)) {
    node.clearChildren();
    node->isLeaf = true;
  } else {
    std::ranges::for_each(node.children(), setFilledNodesToLeaf);
  }
}

SparseVoxelOctree rawTreeToSVO(Tree<TemporaryTreeNode> &tree) {
  if (!tree.hasRoot()) { return SparseVoxelOctree(); }
  tree_traversal::depthFirst(tree, [](Node<TemporaryTreeNode> &node) { node.sortChildren(std::less<>()); });
#if MINIMISE_TREE == 1
  std::ranges::for_each(tree.getRoot().children(), setFilledNodesToLeaf);
#endif
  // TODO:
  auto childDescriptors = std::vector<ChildDescriptor>();

  const auto &root = tree.getRoot();

  auto rootDescriptor = childDescriptorForNode(root);
  auto childPointer = 1u;
  rootDescriptor.childData.childPointer = childPointer;

  childDescriptors.emplace_back(rootDescriptor);

  if (!root->isLeaf) {
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

std::strong_ordering TemporaryTreeNode::operator<=>(const TemporaryTreeNode &rhs) const {
  if (idx < rhs.idx) { return std::strong_ordering::less; }
  if (idx == rhs.idx) { return std::strong_ordering::equal; }
  return std::strong_ordering::greater;
}
}// namespace details

}// namespace pf::vox