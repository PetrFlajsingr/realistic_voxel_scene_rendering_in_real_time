/**
 * @file RawVoxelModel.cpp
 * @brief Structures to store raw voxel models.
 * @author Petr Flajšingr
 * @date 7.12.20
 */

#include "RawVoxelModel.h"

#include <utility>

namespace pf::vox {

VoxelInfo::VoxelInfo(const glm::vec4 &position, std::uint32_t matId) : position(position), materialId(matId) {}

RawVoxelModel::RawVoxelModel(std::string name, std::vector<VoxelInfo> voxels, glm::ivec3 size)
    : name(std::move(name)), voxels(std::move(voxels)), modelSize(size) {}

const std::string &RawVoxelModel::getName() const { return name; }
const std::vector<VoxelInfo> &RawVoxelModel::getVoxels() const { return voxels; }
}// namespace pf::vox