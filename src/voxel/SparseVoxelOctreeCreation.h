//
// Created by petr on 12/15/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREECREATION_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREECREATION_H

#include "ModelLoading.h"
#include "SparseVoxelOctree.h"
#include "utils/StaticTree.h"
#include <glm/vec3.hpp>
#include <pf_common/math/BoundingBox.h>
#include <utility>

namespace pf::vox {

constexpr auto OCTREE_DEPTH_LIMIT = 64;

SparseVoxelOctree loadFileAsSVO(const std::filesystem::path &srcFile, FileType fileType = FileType::Unknown);

SparseVoxelOctree convertSceneToSVO(const Scene &scene);

namespace details {
struct TemporaryTreeNode {
  bool isValid = false;
  struct {
    std::string position;
  } debug;
};

SparseVoxelOctree loadVoxFileAsSVO(std::ifstream &&istream);

math::BoundingBox<3> findBB(const Scene &scene);

math::BoundingBox<3> bbToOctreeBB(math::BoundingBox<3> bb, uint32_t levels);

uint32_t calcOctreeLevelCount(const math::BoundingBox<3> &bb);

void addVoxelToTree(static_tree::StaticTree<TemporaryTreeNode, 8> &tree, const Voxel &voxel, uint32_t octreeLevels);

SparseVoxelOctree rawTreeToSVO(const static_tree::StaticTree<TemporaryTreeNode, 8> &tree);
}// namespace details

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREECREATION_H
