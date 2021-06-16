//
// Created by petr on 12/7/20.
//

#include "RawVoxelModel.h"

#include <utility>

namespace pf::vox {

VoxelInfo::VoxelInfo(const glm::vec4 &position, const glm::vec4 &color) : position(position), color(color) {}

RawVoxelModel::RawVoxelModel(std::string name, std::vector<VoxelInfo> voxels, glm::ivec3 size)
    : name(std::move(name)), voxels(std::move(voxels)), modelSize(size) {}

const std::string &RawVoxelModel::getName() const { return name; }
const std::vector<VoxelInfo> &RawVoxelModel::getVoxels() const { return voxels; }
}// namespace pf::vox