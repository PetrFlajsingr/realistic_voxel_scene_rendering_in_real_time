/**
 * @file SparseVoxelOctree.cpp
 * @brief Structs for sparse voxel octree.
 * @author Petr Flajšingr
 * @date 15.12.20
 */


#include "SparseVoxelOctree.h"
#include "../../../pf_common/include/pf_common/bin.h"
#include <pf_common/exceptions/StackTraceException.h>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

namespace pf::vox {

using namespace ranges;
/* FIXME: ESVO original version
std::string ChildDescriptor::toString() const {
  const auto uintChildData = *reinterpret_cast<const uint32_t *>(&childData);
  return fmt::format("Child data: (raw){:32b}, valid mask: {:8b}, leaf mask: {:8b}, child pointer: {}", uintChildData,
                     uintChildData >> 8 & 0xFF, uintChildData & 0xFF, childData.childPointer);
}

std::string ChildDescriptor::stringDraw() const {
  const auto uintChildData = *reinterpret_cast<const uint32_t *>(&childData);
  const auto validMask = uintChildData >> 8 & 0xFF;
  auto result = std::string();
  for (uint32_t i = 0; i < 8; ++i) {
    result += (validMask & (1 << i)) ? '#' : 'E';
    if (i % 2 == 1) { result += '\n'; }
  }
  return result;
}*/
std::string ChildDescriptor::toString() const {
  return fmt::format("Child data: valid mask: {:8b}, leaf mask: {:8b}, child pointer: {}", validMask, leafMask,
                     childPointer);
}

std::string ChildDescriptor::stringDraw() const {
  auto result = std::string();
  for (uint32_t i = 0; i < 8; ++i) {
    result += (validMask & (1 << i)) ? '#' : 'E';
    if (i % 2 == 1) { result += '\n'; }
  }
  return result;
}

std::string Page::toString() const {
  const auto descriptorsAsString = childDescriptors | views::enumerate
      | views::transform([](const auto &desc) { return fmt::format("#{:3}: {}", desc.first, desc.second.toString()); })
      | to_vector;

  return descriptorsAsString | views::join('\n') | to<std::string>;
}

/**
 * Total page size 4B
 * Header 4B
 * Child descriptors size 4B
 * Child descriptors
 * Far pointers size 4B
 * Far pointers
 */

std::vector<std::byte> Page::serialize() const {
  const auto rawPageHeader = toBytes(header);
  const auto rawPageDescriptors = std::span(reinterpret_cast<const std::byte *>(childDescriptors.data()),
                                            childDescriptors.size() * sizeof(ChildDescriptor));
  const auto descriptorsSize = static_cast<uint64_t>(rawPageDescriptors.size());
  const auto rawFarPointers =
      std::span(reinterpret_cast<const std::byte *>(farPointers.data()), farPointers.size() * sizeof(uint32_t));
  const auto farPointersSize = static_cast<uint64_t>(rawFarPointers.size());

  const auto size = static_cast<uint64_t>(rawPageHeader.size() + sizeof(descriptorsSize) + descriptorsSize
                                          + sizeof(farPointersSize) + farPointersSize);
  const auto rawSize = toBytes(size);
  const auto rawDescriptorsSize = toBytes(descriptorsSize);
  const auto rawFarPointersSize = toBytes(farPointersSize);
  return views::concat(rawSize, rawPageHeader, rawDescriptorsSize, rawPageDescriptors, rawFarPointersSize,
                       rawFarPointers)
      | to_vector;
}
Page Page::Deserialize(std::span<const std::byte> data) {
  [[maybe_unused]] const auto pageSize = fromBytes<uint64_t>(data.first(8));
  auto result = Page();
  auto offset = sizeof(pageSize);
  result.header = fromBytes<PageHeader>(data.subspan(offset, 8));
  offset += sizeof(result.header);

  const auto childDescriptorsSize = fromBytes<uint64_t>(data.subspan(offset, 8));
  offset += sizeof(childDescriptorsSize);
  const auto childDescriptorCount = childDescriptorsSize / sizeof(ChildDescriptor);
  result.childDescriptors.resize(childDescriptorCount);
  const auto rawDescriptors =
      std::span(reinterpret_cast<const ChildDescriptor *>(data.data() + offset), childDescriptorCount);
  std::ranges::copy(rawDescriptors, result.childDescriptors.begin());
  offset += sizeof(childDescriptorsSize);

  //const auto farPointersSize = fromBytes<uint64_t>(data.subspan(offset, 8));
  //offset += sizeof(farPointersSize);
  //const auto farPointersCount = farPointersSize / sizeof(uint32_t);
  //result.farPointers.resize(farPointersCount);
  //const auto rawFarPointers = std::span(reinterpret_cast<const uint32_t *>(data.data() + offset), farPointersCount);
  //std::ranges::copy(rawFarPointers, result.farPointers.begin());

  return result;
}

/**
 * Total size 4B
 * Info section size 4B
 * Info section
 * Pages
 */

std::vector<std::byte> Block::serialize() const {
  const auto rawPages = pages | views::transform([](const auto &page) { return page.serialize(); }) | to_vector;
  const auto rawInfoSectionAttachments =
      std::span(reinterpret_cast<const std::byte *>(infoSection.attachments.attachments.data()),
                infoSection.attachments.attachments.size() * sizeof(MaterialIndexAttachment));
  //infoSection.attachments.attachments.size() * sizeof(PhongAttachment));
  const auto rawInfoSectionLookupEntries =
      std::span(reinterpret_cast<const std::byte *>(infoSection.attachments.lookupEntries.data()),
                infoSection.attachments.lookupEntries.size() * sizeof(AttachmentLookupEntry));
  const auto pagesSize = static_cast<uint64_t>(
      accumulate(rawPages | views::transform([](const auto &rawPage) { return rawPage.size(); }), 0));
  const auto infoSectionAttachmentsSize = static_cast<uint64_t>(rawInfoSectionAttachments.size());
  const auto rawInfoSectionAttachmentsSize = toBytes(infoSectionAttachmentsSize);
  const auto lookupEntriesSize = static_cast<uint64_t>(rawInfoSectionLookupEntries.size());
  const auto rawInfoSectionLookupEntriesSize = toBytes(lookupEntriesSize);

  const auto rawSize = toBytes(pagesSize + infoSectionAttachmentsSize + lookupEntriesSize + 2 * sizeof(uint64_t));

  return views::concat(rawSize, rawInfoSectionAttachmentsSize, rawInfoSectionAttachments,
                       rawInfoSectionLookupEntriesSize, rawInfoSectionLookupEntries, rawPages | views::join)
      | to_vector;
}

Block Block::Deserialize(std::span<const std::byte> data) {
  const auto blockSize = fromBytes<uint64_t>(data.first(8));
  auto result = Block();
  auto offset = sizeof(blockSize);
  const auto infoSectionAttachmentsSize = fromBytes<uint64_t>(data.subspan(offset, 8));
  offset += 8;

  const auto attachmentCount = infoSectionAttachmentsSize / sizeof(MaterialIndexAttachment);
  result.infoSection.attachments.attachments.resize(attachmentCount);
  auto infoSectionAttachmentsSpan =
      std::span(reinterpret_cast<const MaterialIndexAttachment *>(data.data() + offset), attachmentCount);
  std::ranges::copy(infoSectionAttachmentsSpan, result.infoSection.attachments.attachments.begin());

  offset += infoSectionAttachmentsSize;

  const auto infoSectionLookupEntriesSize = fromBytes<uint64_t>(data.subspan(offset, 8));
  offset += 8;

  const auto lookupEntryCount = infoSectionLookupEntriesSize / sizeof(AttachmentLookupEntry);
  result.infoSection.attachments.lookupEntries.resize(lookupEntryCount);
  auto infoSectionLookupEntrySpan =
      std::span(reinterpret_cast<const AttachmentLookupEntry *>(data.data() + offset), lookupEntryCount);
  std::ranges::copy(infoSectionLookupEntrySpan, result.infoSection.attachments.lookupEntries.begin());

  offset += infoSectionLookupEntriesSize;

  while (offset < blockSize + sizeof(blockSize)) {
    const auto pageSpan = data.subspan(offset);
    const auto pageSize = fromBytes<uint64_t>(pageSpan.first(8));
    result.pages.emplace_back(Page::Deserialize(pageSpan));
    [[maybe_unused]] const auto oldOffset = offset;
    offset += sizeof(pageSize) + pageSize;
    assert(offset != oldOffset);
  }
  return result;
}

SparseVoxelOctree::SparseVoxelOctree(std::vector<Block> &&blocks) : blocks(std::move(blocks)) {}

void SparseVoxelOctree::addBlock(Block &&block) { blocks.emplace_back(std::move(block)); }

std::vector<std::byte> SparseVoxelOctree::serialize() const {
  const auto rawBlocks = blocks | views::transform([](const auto &block) { return block.serialize(); }) | to_vector;
  const auto totalSize = static_cast<uint64_t>(
      accumulate(rawBlocks | views::transform([](const auto &rawBlock) { return rawBlock.size(); }), 0));
  const auto rawSize = toBytes(totalSize);

  return views::concat(rawSize, rawBlocks | views::join) | to_vector;
}
SparseVoxelOctree SparseVoxelOctree::Deserialize(std::span<const std::byte> data) {
  const auto totalSize = fromBytes<uint64_t>(data.first(8));
  auto result = SparseVoxelOctree();
  auto offset = sizeof(totalSize);
  while (offset < totalSize + sizeof(totalSize)) {
    const auto blockSpan = data.subspan(offset);
    const auto blockSize = fromBytes<uint64_t>(blockSpan.first(8));
    result.blocks.emplace_back(Block::Deserialize(blockSpan));
    [[maybe_unused]] const auto oldOffset = offset;
    offset += sizeof(blockSize) + blockSize;
    assert(offset != oldOffset);
  }
  return result;
}

const std::vector<Block> &SparseVoxelOctree::getBlocks() const { return blocks; }
}// namespace pf::vox