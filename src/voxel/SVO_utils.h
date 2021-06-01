//
// Created by petr on 1/5/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SVO_UTILS_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SVO_UTILS_H

#include "SparseVoxelOctree.h"
#include <pf_common/bin.h>
#include <pf_glfw_vulkan/vulkan/types/Buffer.h>

namespace pf::vox {
inline void copySvoToBuffer(const SparseVoxelOctree &svo, vulkan::BufferMapping &mapping) {
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
inline tl::expected<vulkan::BufferMemoryPool<4>::Block, std::string>
copySvoToMemoryBlock([[maybe_unused]] const SparseVoxelOctree &svo,
                     [[maybe_unused]] vulkan::BufferMemoryPool<4> &memoryPool) {
  auto data = toBytes(svo.getBlocks()[0].pages[0].header);
  auto addData([&data](const auto &newData) { std::ranges::copy(newData, std::back_inserter(data)); });

  const auto &descriptors = svo.getBlocks()[0].pages[0].childDescriptors;
  addData(
      std::span(reinterpret_cast<const std::byte *>(descriptors.data()), descriptors.size() * sizeof(ChildDescriptor)));

  const auto &lookups = svo.getBlocks()[0].infoSection.attachments.lookupEntries;
  addData(
      std::span(reinterpret_cast<const std::byte *>(lookups.data()), lookups.size() * sizeof(AttachmentLookupEntry)));

  const auto &attachments = svo.getBlocks()[0].infoSection.attachments.attachments;
  addData(
      std::span(reinterpret_cast<const std::byte *>(attachments.data()), attachments.size() * sizeof(PhongAttachment)));
  auto memBlock = memoryPool.leaseMemory(data.size());
  auto mapping = memBlock->mapping();
  mapping.set(data);
  return memBlock;
}
}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SVO_UTILS_H
