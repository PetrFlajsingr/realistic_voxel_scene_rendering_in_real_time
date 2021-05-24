//
// Created by petr on 12/15/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREECREATION_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREECREATION_H

#include "ModelLoading.h"
#include "SparseVoxelOctree.h"
#include <glm/vec3.hpp>
#include <pf_common/Tree.h>
#include <pf_common/math/BoundingBox.h>
#include <utility>

namespace pf::vox {

constexpr auto OCTREE_DEPTH_LIMIT = 64;

struct SparseVoxelOctreeCreateInfo {
  uint32_t depth;
  uint32_t initVoxelCount;
  uint32_t voxelCount;
};

std::pair<SparseVoxelOctree, SparseVoxelOctreeCreateInfo> loadFileAsSVO(const std::filesystem::path &srcFile,
                                                                        FileType fileType = FileType::Unknown);

std::pair<SparseVoxelOctree, SparseVoxelOctreeCreateInfo> convertSceneToSVO(const Scene &scene);

namespace details {
struct TemporaryTreeNode {
  bool isLeaf;
  uint32_t idx;
  glm::vec4 color;
  uint32_t paletteIdx;
  struct {
    std::string position;
  } debug;
  std::strong_ordering operator<=>(const TemporaryTreeNode &rhs) const;
};

std::pair<SparseVoxelOctree, SparseVoxelOctreeCreateInfo> loadVoxFileAsSVO(std::ifstream &&istream);

math::BoundingBox<3> findBB(const Scene &scene);

math::BoundingBox<3> bbToOctreeBB(math::BoundingBox<3> bb, uint32_t levels);

uint32_t calcOctreeLevelCount(const math::BoundingBox<3> &bb);

void addVoxelToTree(Tree<TemporaryTreeNode> &tree, const Voxel &voxel, uint32_t octreeLevels);

std::pair<SparseVoxelOctree, uint32_t> rawTreeToSVO(Tree<TemporaryTreeNode> &tree);
}// namespace details

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREECREATION_H
