/**
 * @file SparseVoxelOctreeCreation.h
 * @brief Functions for loading voxel data and transforming them into sparse voxel octree.
 * @author Petr Flajšingr
 * @date 15.12.20
 */


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

/**
 * @brief Information about the created octree, including the octree's data.
 */
struct SparseVoxelOctreeCreateInfo {
  uint32_t depth;
  uint32_t initVoxelCount;
  uint32_t voxelCount;
  math::BoundingBox<3> AABB;
  SparseVoxelOctree data;
  glm::vec3 center;
  std::vector<MaterialProperties> materials;
};

/**
 * Load a file as an SVO.
 * @param srcFile path to the source file
 * @param sceneAsOneSVO if true then each submodel in the scene will be returned as its own SVO
 * @param fileType type of the file, if Unknown then the function will attempt to detect it
 * @return vector of SVOs created from the file
 */
std::vector<SparseVoxelOctreeCreateInfo> loadFileAsSVO(const std::filesystem::path &srcFile, bool sceneAsOneSVO,
                                                       FileType fileType = FileType::Unknown);

/**
 * Convert raw scene data to an SVO with a possibility to load each model as its own SVO.
 * @param scene source data
 * @param sceneAsOneSVO if true all models are combined into one SVO
 * @return vector of created SVOs
 */
std::vector<SparseVoxelOctreeCreateInfo> convertSceneToSVO(const RawVoxelScene &scene, bool sceneAsOneSVO);
/**
 * Convert raw voxel model into an SVO.
 * @param model source data
 * @return model as SVO
 */
SparseVoxelOctreeCreateInfo convertModelToSVO(const RawVoxelModel &model);

namespace details {
/**
 * @brief Intermediate tree node for SVO conversion.
 */
struct TemporaryTreeNode {
  bool isLeaf;
  std::uint32_t idx;
  std::uint32_t materialId;
  std::uint32_t paletteIdx;
  struct {
    std::string position;
  } debug;
  std::strong_ordering operator<=>(const TemporaryTreeNode &rhs) const;
};
/**
 * Load .VOX file and convert it to SVOs.
 * @param istream source data
 * @param sceneAsOneSVO if true all models are converted into one SVO
 * @return vector of converted SVOs
 */
std::vector<SparseVoxelOctreeCreateInfo> loadVoxFileAsSVO(std::ifstream &&istream, bool sceneAsOneSVO);
/**
 * Load .PF_VOX file and convert it to SVOs.
 * @param istream source data
 * @return vector of converted SVOs
 */
std::vector<SparseVoxelOctreeCreateInfo> loadPfVoxFileAsSVO(std::ifstream &&istream);
/**
 * Find a bounding box for the entire scene.
 * @param scene source data
 * @return bounding box of the scene
 */
math::BoundingBox<3> findSceneBB(const RawVoxelScene &scene);
/**
 * Find a bounding box of a raw model.
 * @param model source data
 * @return
 */
math::BoundingBox<3> findModelBB(const RawVoxelModel &model);
/**
 * Convert scene bounding box to a bounding box covering the whole octree space.
 * @param bb starting bounding box
 * @param levels depth of the tree
 * @return tree's bounding box
 */
math::BoundingBox<3> bbToOctreeBB(math::BoundingBox<3> bb, uint32_t levels);
/**
 * Calculate octree depth based on its bounding box.
 * @param bb bounding box
 * @return depth of the tree
 */
uint32_t calcOctreeLevelCount(const math::BoundingBox<3> &bb);
/**
 * Push voxel into a tree, creating additional nodes as needed.
 * @param tree tree to push into
 * @param voxel info on voxel
 * @param octreeLevels depth of the tree
 */
void addVoxelToTree(Tree<TemporaryTreeNode> &tree, const VoxelInfo &voxel, uint32_t octreeLevels);
/**
 * Convert intermediate tree into an SVO.
 * @param tree source tree
 * @return octree data and count of voxels after minimisation
 */
std::pair<SparseVoxelOctree, uint32_t> rawTreeToSVO(Tree<TemporaryTreeNode> &tree);
}// namespace details

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SPARSEVOXELOCTREECREATION_H
