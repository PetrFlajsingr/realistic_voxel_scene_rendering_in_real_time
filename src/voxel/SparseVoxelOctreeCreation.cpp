//
// Created by petr on 12/15/20.
//

#include "SparseVoxelOctreeCreation.h"
#include "SparseVoxelOctree.h"
#include <fstream>
#include <logging/loggers.h>
#include <magic_enum.hpp>
#include <pf_common/bin.h>
#include <pf_common/bits.h>
#include <range/v3/action/sort.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#define MINIMISE_TREE 1

namespace pf::vox {
using namespace ranges;

std::vector<SparseVoxelOctreeCreateInfo> loadFileAsSVO(const std::filesystem::path &srcFile, bool sceneAsOneSVO,
                                                       FileType fileType) {
  //logd("VOX", "Loading file: {}", srcFile.string());
  if (fileType == FileType::Unknown) {
    const auto detectedFileType = details::detectFileType(srcFile);
    if (!detectedFileType.has_value()) { throw LoadException("Could not detect file type for '{}'", srcFile.string()); }
    fileType = *detectedFileType;
  }
  auto ifstream = std::ifstream(srcFile, std::ios::binary);
  if (!ifstream.is_open()) { throw LoadException("Could not open file '{}'", srcFile.string()); }
  switch (fileType) {
    case FileType::Vox: return details::loadVoxFileAsSVO(std::move(ifstream), sceneAsOneSVO); break;
    case FileType::PfVox: return details::loadPfVoxFileAsSVO(std::move(ifstream)); break;
    default:
      throw LoadException("Could not load model '{}', unsupported format: {}", srcFile.string(),
                          magic_enum::enum_name(fileType));
  }
}

SparseVoxelOctreeCreateInfo convertSceneToSVO(const RawVoxelScene &scene) {
  auto bb = details::findSceneBB(scene);
  //logd("VOX", "Found BB");
  const auto octreeLevels = details::calcOctreeLevelCount(bb);
  //logd("VOX", "Octree level count: {}", octreeLevels);
  auto voxels = scene.getModels() | views::transform([](const auto &model) { return model->getVoxels() | views::all; })
      | views::join | to_vector | actions::sort([](const auto &a, const auto &b) {
                  return a.position.x < b.position.x && a.position.y < b.position.y && a.position.z < b.position.z;
                });
  //logd("VOX", "Voxel count: {}", voxels.size());

  auto tree = Tree<details::TemporaryTreeNode>();

  std::ranges::for_each(
      voxels, [&tree, octreeLevels](const auto &voxel) { details::addVoxelToTree(tree, voxel, octreeLevels); });
  //logd("VOX", "Built intermediate tree");
  const auto octreeSizeLength = std::pow(2, octreeLevels);
  const auto bbDiff = (bb.p2 - bb.p1) / static_cast<float>(octreeSizeLength);
  bb.p2 = bb.p1 + bbDiff;
  auto resultTree = rawTreeToSVO(tree);
  auto createInfo =
      SparseVoxelOctreeCreateInfo{octreeLevels, static_cast<uint32_t>(voxels.size()), 0,
                                  bb,           std::move(resultTree.first),          scene.getSceneCenter().xzy()};
  createInfo.voxelCount = resultTree.second == 0 ? createInfo.initVoxelCount : resultTree.second;
  return createInfo;
}

std::vector<SparseVoxelOctreeCreateInfo> convertSceneToSVO(const RawVoxelScene &scene, bool sceneAsOneSVO) {
  if (sceneAsOneSVO) {
    auto bb = details::findSceneBB(scene);
    //logd("VOX", "Found BB");
    const auto octreeLevels = details::calcOctreeLevelCount(bb);
    //logd("VOX", "Octree level count: {}", octreeLevels);
    auto voxels = scene.getModels()
        | views::transform([](const auto &model) { return model->getVoxels() | views::all; }) | views::join | to_vector
        | actions::sort([](const auto &a, const auto &b) {
                    return a.position.x < b.position.x && a.position.y < b.position.y && a.position.z < b.position.z;
                  });
    //logd("VOX", "Voxel count: {}", voxels.size());

    auto tree = Tree<details::TemporaryTreeNode>();

    std::ranges::for_each(
        voxels, [&tree, octreeLevels](const auto &voxel) { details::addVoxelToTree(tree, voxel, octreeLevels); });
    //logd("VOX", "Built intermediate tree");0
    const auto octreeSizeLength = std::pow(2, octreeLevels);
    const auto bbDiff = (bb.p2 - bb.p1) / static_cast<float>(octreeSizeLength);
    bb.p2 = bb.p1 + bbDiff;
    auto resultTree = rawTreeToSVO(tree);
    auto createInfo =
        SparseVoxelOctreeCreateInfo{octreeLevels, static_cast<uint32_t>(voxels.size()), 0,
                                    bb,           std::move(resultTree.first),          scene.getSceneCenter().xzy()};
    createInfo.voxelCount = resultTree.second == 0 ? createInfo.initVoxelCount : resultTree.second;
    return {createInfo};
  } else {
    return scene.getModels() | views::transform([&](const auto &model) {
             auto result = convertModelToSVO(*model);
             result.center = scene.getSceneCenter().xzy();
             return result;
           })
        | ranges::to_vector;
  }
}
SparseVoxelOctreeCreateInfo convertModelToSVO(const RawVoxelModel &model) {
  auto bb = details::findModelBB(model);
  //logd("VOX", "Found BB");
  const auto octreeLevels = details::calcOctreeLevelCount(bb);
  //logd("VOX", "Octree level count: {}", octreeLevels);
  auto voxels = model.getVoxels() | to_vector | actions::sort([](const auto &a, const auto &b) {
                  return a.position.x < b.position.x && a.position.y < b.position.y && a.position.z < b.position.z;
                });
  //logd("VOX", "Voxel count: {}", voxels.size());

  auto tree = Tree<details::TemporaryTreeNode>();

  std::ranges::for_each(
      voxels, [&tree, octreeLevels](const auto &voxel) { details::addVoxelToTree(tree, voxel, octreeLevels); });
  //logd("VOX", "Built intermediate tree");
  const auto octreeSizeLength = std::pow(2, octreeLevels);
  const auto bbDiff = (bb.p2 - bb.p1) / static_cast<float>(octreeSizeLength);
  bb.p2 = bb.p1 + bbDiff;
  auto resultTree = rawTreeToSVO(tree);
  auto createInfo = SparseVoxelOctreeCreateInfo{octreeLevels, static_cast<uint32_t>(voxels.size()), 0,
                                                bb,           std::move(resultTree.first),          glm::vec3{}};
  createInfo.voxelCount = resultTree.second == 0 ? createInfo.initVoxelCount : resultTree.second;
  return createInfo;
}

namespace details {
std::vector<SparseVoxelOctreeCreateInfo> loadVoxFileAsSVO(std::ifstream &&istream, bool sceneAsOneSVO) {
  const auto scene = loadVoxScene(std::move(istream));
  //logd("VOX", "Loaded scene");
  return convertSceneToSVO(scene, sceneAsOneSVO);
}

std::vector<SparseVoxelOctreeCreateInfo> loadPfVoxFileAsSVO(std::ifstream &&istream) {
  auto data = std::vector<char>((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());
  auto dataView = std::span{reinterpret_cast<const std::byte *>(data.data()), data.size()};
  const auto svoVoxelCount = fromBytes<uint32_t>(dataView.first(sizeof(uint32_t)));
  const auto svoDepth = fromBytes<uint32_t>(dataView.subspan(sizeof(uint32_t), sizeof(uint32_t)));
  const auto aabb =
      fromBytes<math::BoundingBox<3>>(dataView.subspan(sizeof(uint32_t) * 2, sizeof(math::BoundingBox<3>)));
  const auto svo = SparseVoxelOctree::Deserialize(
      dataView.subspan(sizeof(uint32_t) * 2 + sizeof(math::BoundingBox<3>),
                       dataView.size() - (sizeof(uint32_t) * 2) + sizeof(math::BoundingBox<3>)));
  return {SparseVoxelOctreeCreateInfo{svoDepth, svoVoxelCount, svoVoxelCount, aabb, std::move(svo), glm::vec3{}}};
}

math::BoundingBox<3> findSceneBB(const RawVoxelScene &scene) {
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

math::BoundingBox<3> findModelBB(const RawVoxelModel &model) {
  constexpr auto MIN = std::numeric_limits<float>::lowest();

  auto result = math::BoundingBox<3>(glm::vec3{0}, glm::vec3{MIN});

  std::ranges::for_each(model.getVoxels(), ([&result](const auto &voxel) {
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
  throw LoadException("Octree depth higher than limit '{}'", OCTREE_DEPTH_LIMIT);
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

void addVoxelToTree(Tree<TemporaryTreeNode> &tree, const VoxelInfo &voxel, uint32_t octreeLevels) {
  if (!tree.hasRoot()) { tree.initRoot(TemporaryTreeNode{}); }
  auto node = &tree.getRoot();
  (*node)->idx = 0;
  (*node)->isLeaf = false;
  for (uint32_t i = 0; i < octreeLevels; ++i) {
    const auto idx = idxForLevel(voxel.position.xyz(), i, octreeLevels);
    auto children = node->children();
    auto childNodeIter = std::ranges::find_if(children, [idx](const auto &child) { return child->idx == idx; });
    Node<TemporaryTreeNode> *childNode;
    if (childNodeIter == children.end()) {
      childNode = &node->appendChild();
      (*childNode)->idx = idx;
    } else {
      childNode = &*childNodeIter;
    }
    (*childNode)->color = voxel.color;
    (*childNode)->debug.position = fmt::format("{}x{}x{}", voxel.position.x, voxel.position.y, voxel.position.z);
    node = childNode;
  }
  (*node)->isLeaf = true;
}

std::size_t countNonLeafChildrenForNode(const Node<TemporaryTreeNode> &node) {
  return std::ranges::count_if(node.children(), [](const auto &child) { return !child->isLeaf; });
}

auto getValidChildren(const Node<TemporaryTreeNode> &node) {
  return node.children() | views::filter([](const auto &child) { return !child->isLeaf; })
      | views::transform([](const auto &child) { return &child; });
}

auto getLeafChildren(const Node<TemporaryTreeNode> &node) {
  return node.children() | views::filter([](const auto &child) { return child->isLeaf; })
      | views::transform([](const auto &child) { return &child; });
}

auto getValidChildren(const std::vector<const Node<TemporaryTreeNode> *> &nodes) {
  return nodes | views::transform([](const auto &node) { return getValidChildren(*node); }) | views::join;
}

ChildDescriptor childDescriptorForNode(const Node<TemporaryTreeNode> &node) {
  auto result = ChildDescriptor();
  //result.childData.far = 0;
  result.leafMask = 0;
  result.validMask = 0;
  for (const auto &child : node.children()) {
    result.validMask |= 1u << child->idx;
    if (child->isLeaf) { result.leafMask |= 1u << child->idx; }
  }
  return result;
}

std::vector<ChildDescriptor> buildDescriptors(const std::vector<const Node<TemporaryTreeNode> *> &nodes) {
  auto result = nodes | views::transform([](const auto &node) { return childDescriptorForNode(*node); }) | to_vector;

  auto nodesWithDescriptors = views::zip(nodes, result);

  auto iter = nodesWithDescriptors.begin();
  const auto end = nodesWithDescriptors.end();
  if (iter != end) {
    auto offset = std::distance(iter, end);
    const auto &[child, descriptor] = *iter;
    descriptor.childPointer = offset;
    ++iter;
    for (; iter != end; ++iter) {
      --offset;
      const auto &[previousChild, _] = *(iter - 1);
      const auto &[child, descriptor] = *iter;
      offset += countNonLeafChildrenForNode(*previousChild);
      //if (offset > 32767) {
      //  //descriptor.childData.far = 1;
      //  throw StackTraceException("Far pointers not yet implemented");
      //}
      descriptor.childPointer = offset;
    }
  }
  auto validChildren = getValidChildren(nodes) | to_vector;
  if (!validChildren.empty()) {
    const auto descriptors = buildDescriptors(validChildren);
    const auto originalResultSize = result.size();
    result.resize(originalResultSize + descriptors.size());
    std::ranges::copy(descriptors, result.begin() + originalResultSize);
  }
  return result;
}

AttachmentLookupEntry attLookupEntryForNode(const Node<TemporaryTreeNode> &node) {
  auto result = AttachmentLookupEntry();
  result.mask = 0;
  result.valuePointer = 0;
  for (const auto &child : node.children()) {
    if (child->isLeaf) { result.mask |= 1u << child->idx; }
  }
  return result;
}

PhongAttachment attachmentForNode(const Node<TemporaryTreeNode> &node) {
  constexpr auto COLOR_MULTIPLIER = 255.f;
  return PhongAttachment{.color = {.alpha = static_cast<uint8_t>(node->color.a * COLOR_MULTIPLIER),
                                   .blue = static_cast<uint8_t>(node->color.b * COLOR_MULTIPLIER),
                                   .green = static_cast<uint8_t>(node->color.g * COLOR_MULTIPLIER),
                                   .red = static_cast<uint8_t>(node->color.r * COLOR_MULTIPLIER)}};
}
std::pair<AttachmentLookupEntry, std::vector<PhongAttachment>> attDataForNode(const Node<TemporaryTreeNode> &node) {
  auto lookupEntry = attLookupEntryForNode(node);
  auto attachments =
      getLeafChildren(node) | views::transform([](const auto &child) { return attachmentForNode(*child); }) | to_vector;
  return {lookupEntry, attachments};
}

std::pair<std::vector<AttachmentLookupEntry>, std::vector<PhongAttachment>>
buildAttLookupEntriesWithAttachments(const std::vector<const Node<TemporaryTreeNode> *> &nodes, uint32_t &attOffset) {
  auto resultLookups = std::vector<AttachmentLookupEntry>();
  auto resultAttachments = std::vector<PhongAttachment>();
  for (const auto &node : nodes) {
    auto [lookupEntry, attachments] = attDataForNode(*node);
    if (!attachments.empty()) {
      lookupEntry.valuePointer = attOffset;
      attOffset += attachments.size();
      std::ranges::copy(attachments, std::back_inserter(resultAttachments));
    }
    resultLookups.emplace_back(lookupEntry);
  }

  auto validChildren = getValidChildren(nodes) | to_vector;
  if (!validChildren.empty()) {
    const auto [lookupEntries, attachments] = buildAttLookupEntriesWithAttachments(validChildren, attOffset);
    std::ranges::copy(lookupEntries, std::back_inserter(resultLookups));
    std::ranges::copy(attachments, std::back_inserter(resultAttachments));
  }
  return {resultLookups, resultAttachments};
}

bool isNodeFilled(const Node<TemporaryTreeNode> &node) {
  if (node->isLeaf) { return true; }
  return node.childrenSize() == 8 && std::ranges::all_of(node.children(), isNodeFilled);
}

void setFilledNodesToLeaf(Node<TemporaryTreeNode> &node) {
  if (node->isLeaf) { return; }
  if (isNodeFilled(node)) {
    auto children = node.children();
    const auto colorOfFirstChild = children[0]->color;
    if (std::ranges::all_of(children,
                            [colorOfFirstChild](const auto &child) { return child->color == colorOfFirstChild; })) {
      node.clearChildren();
      node->isLeaf = true;
      node->color = colorOfFirstChild;
    }
  } else {
    std::ranges::for_each(node.children(), setFilledNodesToLeaf);
  }
}

std::pair<SparseVoxelOctree, uint32_t> rawTreeToSVO(Tree<TemporaryTreeNode> &tree) {
  if (!tree.hasRoot()) { return {SparseVoxelOctree(), 0}; }
  std::ranges::for_each(tree.iterNodesBreadthFirst(), [](auto &node) { node.sortChildren(std::less<>()); });
  //logd("VOX", "Sorted child nodes");
  auto minimisedCount = 0;
#if MINIMISE_TREE == 1
  std::ranges::for_each(tree.getRoot().children(), setFilledNodesToLeaf);
  const auto leafCount =
      std::ranges::count_if(tree.iterBreadthFirst(), [](const auto &record) { return record.isLeaf; });
  minimisedCount = leafCount;
  //logd("VOX", "Minimised tree, remaining leaf voxels: {}", leafCount);
#endif

  auto childDescriptors = std::vector<ChildDescriptor>();

  auto attLookups = std::vector<AttachmentLookupEntry>();
  auto attachments = std::vector<PhongAttachment>();

  const auto &root = tree.getRoot();
  auto rootDescriptor = childDescriptorForNode(root);
  rootDescriptor.childPointer = 1u;

  childDescriptors.emplace_back(rootDescriptor);

  const auto [rootLookup, rootAttachments] = attDataForNode(root);
  uint32_t attOffset = rootAttachments.size();
  attLookups.emplace_back(rootLookup);
  std::ranges::copy(rootAttachments, std::back_inserter(attachments));

  if (!root->isLeaf) {
    const auto validChildren = getValidChildren(root) | to_vector;
    const auto descriptors = buildDescriptors(validChildren);
    childDescriptors.resize(childDescriptors.size() + descriptors.size());
    std::ranges::copy(descriptors, childDescriptors.begin() + 1);

    const auto [childLookups, childAttachments] = buildAttLookupEntriesWithAttachments(validChildren, attOffset);
    std::ranges::copy(childLookups, std::back_inserter(attLookups));
    std::ranges::copy(childAttachments, std::back_inserter(attachments));
  }

  //logd("VOX", "Assembling SVO");

  auto blockAttachments = Attachments();
  blockAttachments.lookupEntries = attLookups;
  blockAttachments.attachments = attachments;

  auto page = Page();
  page.childDescriptors = std::move(childDescriptors);
  page.header.infoSectionPointer = page.childDescriptors.size() * 2;
  page.header.attachmentsPointer = page.childDescriptors.size() * 2 + blockAttachments.lookupEntries.size();
  page.farPointers = {};

  auto block = Block();
  block.pages = {std::move(page)};
  block.infoSection.attachments = blockAttachments;

  //logd("VOX", "SVO build done");
  return {SparseVoxelOctree({std::move(block)}), minimisedCount};
}
std::strong_ordering TemporaryTreeNode::operator<=>(const TemporaryTreeNode &rhs) const {
  if (idx < rhs.idx) { return std::strong_ordering::less; }
  if (idx == rhs.idx) { return std::strong_ordering::equal; }
  return std::strong_ordering::greater;
}

}// namespace details

}// namespace pf::vox