//
// Created by petr on 1/5/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SVO_UTILS_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SVO_UTILS_H

#include <pf_glfw_vulkan/vulkan/types/Buffer.h>
#include "SparseVoxelOctree.h"

namespace pf::vox{
void copySvoToBuffer(const SparseVoxelOctree &svo, vulkan::BufferMapping mapping) {
  mapping.data<PageHeader>()[0] = svo.getBlocks()[0].pages[0].header;
  mapping.setRawOffset(svo.getBlocks()[0].pages[0].childDescriptors, sizeof(PageHeader));
}
}

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SVO_UTILS_H
