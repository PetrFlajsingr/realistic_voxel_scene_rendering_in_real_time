//
// Created by petr on 12/7/20.
//

#include "Model.h"

#include <utility>

namespace pf::vox {

Voxel::Voxel(const glm::vec4 &position, const glm::vec4 &color) : position(position), color(color) {}

Model::Model(std::string name, std::vector<Voxel> voxels) : name(std::move(name)), voxels(std::move(voxels)) {}

const std::string &Model::getName() const { return name; }
const std::vector<Voxel> &Model::getVoxels() const { return voxels; }
}// namespace pf::vox