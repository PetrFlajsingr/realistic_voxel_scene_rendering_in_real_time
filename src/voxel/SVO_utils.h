//
// Created by petr on 1/5/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SVO_UTILS_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SVO_UTILS_H

#include "SparseVoxelOctree.h"
#include <pf_glfw_vulkan/vulkan/types/Buffer.h>

namespace pf::vox {
void copySvoToBuffer(const SparseVoxelOctree &svo, vulkan::BufferMapping mapping) {
  mapping.data<PageHeader>()[0] = svo.getBlocks()[0].pages[0].header;
  const auto &descriptors = svo.getBlocks()[0].pages[0].childDescriptors;
  const auto descriptorsOffset = sizeof(PageHeader);
  mapping.setRawOffset(descriptors, descriptorsOffset);
  const auto &lookups = svo.getBlocks()[0].infoSection.attachments.lookupEntries;
  const auto lookupsOffset = sizeof(PageHeader) + sizeof(ChildDescriptor) * descriptors.size();
  mapping.setRawOffset(lookups, lookupsOffset);
  const auto &attachments = svo.getBlocks()[0].infoSection.attachments.attachments;
  const auto attachmentsOffset = sizeof(PageHeader) + sizeof(ChildDescriptor) * descriptors.size()
      + sizeof(AttachmentLookupEntry) * lookups.size();
  mapping.setRawOffset(attachments, attachmentsOffset);
}
}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SVO_UTILS_H
