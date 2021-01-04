//
// Created by petr on 12/15/20.
//

#include "SparseVoxelOctree.h"
#include <pf_common/exceptions/StackTraceException.h>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>
#include <utils/bin.h>

namespace pf::vox {

using namespace ranges;

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
  const auto descriptorsSize = static_cast<uint32_t>(rawPageDescriptors.size());
  const auto rawFarPointers =
      std::span(reinterpret_cast<const std::byte *>(farPointers.data()), farPointers.size() * sizeof(uint32_t));
  const auto farPointersSize = static_cast<uint32_t>(rawFarPointers.size());

  const auto size = static_cast<uint32_t>(rawPageHeader.size() + sizeof(descriptorsSize) + descriptorsSize
                                          + sizeof(farPointersSize) + farPointersSize);
  const auto rawSize = toBytes(size);
  const auto rawDescriptorsSize = toBytes(descriptorsSize);
  const auto rawFarPointersSize = toBytes(farPointersSize);
  return views::concat(rawSize, rawPageHeader, rawDescriptorsSize, rawPageDescriptors, rawFarPointersSize,
                       rawFarPointers)
      | to_vector;
}

Page Page::Deserialize(std::span<const std::byte> data) {
  [[maybe_unused]] const auto pageSize = fromBytes<uint32_t>(data.first(4));
  auto result = Page();
  auto offset = sizeof(pageSize);
  result.header = fromBytes<PageHeader>(data.subspan(offset, 4));
  offset += sizeof(result.header);

  const auto childDescriptorsSize = fromBytes<uint32_t>(data.subspan(offset, 4));
  offset += sizeof(childDescriptorsSize);
  const auto childDescriptorCount = childDescriptorsSize / sizeof(ChildDescriptor);
  result.childDescriptors.resize(childDescriptorCount);
  const auto rawDescriptors =
      std::span(reinterpret_cast<const ChildDescriptor *>(data.data() + offset), childDescriptorCount);
  std::ranges::copy(rawDescriptors, result.childDescriptors.begin());
  offset += sizeof(childDescriptorsSize);

  const auto farPointersSize = fromBytes<uint32_t>(data.subspan(offset, 4));
  offset += sizeof(farPointersSize);
  const auto farPointersCount = farPointersSize / sizeof(uint32_t);
  result.farPointers.resize(farPointersCount);
  const auto rawFarPointers = std::span(reinterpret_cast<const uint32_t *>(data.data() + offset), farPointersCount);
  std::ranges::copy(rawFarPointers, result.farPointers.begin());

  return result;
}

/**
 * Total size 4B
 * TODO: info section
 * Pages
 */

std::vector<std::byte> Block::serialize() const {
  // TODO: info section
  //const auto rawInfoSection = toBytes(infoSection);
  const auto rawPages = pages | views::transform([](const auto &page) { return page.serialize(); }) | to_vector;
  //const auto infoSectionSize = static_cast<uint32_t>(rawInfoSection.size());
  const auto pagesSize = static_cast<uint32_t>(
      accumulate(rawPages | views::transform([](const auto &rawPage) { return rawPage.size(); }), 0));

  const auto rawSize = toBytes(/*infoSectionSize +*/ pagesSize);

  return views::concat(rawSize, rawPages | views::join) | to_vector;
}

Block Block::Deserialize(std::span<const std::byte> data) {
  // TODO: info section
  const auto blockSize = fromBytes<uint32_t>(data.first(4));
  auto result = Block();
  auto offset = sizeof(blockSize);
  while (offset < blockSize + sizeof(blockSize)) {
    const auto pageSpan = data.subspan(offset);
    const auto pageSize = fromBytes<uint32_t>(pageSpan.first(4));
    result.pages.emplace_back(Page::Deserialize(pageSpan));
    const auto oldOffset = offset;
    offset += sizeof(pageSize) + pageSize;
    assert(offset != oldOffset);
  }
  return result;
}

SparseVoxelOctree::SparseVoxelOctree(std::vector<Block> &&blocks) : blocks(std::move(blocks)) {}

void SparseVoxelOctree::addBlock(Block &&block) { blocks.emplace_back(std::move(block)); }

std::vector<std::byte> SparseVoxelOctree::serialize() const {
  const auto rawBlocks = blocks | views::transform([](const auto &block) { return block.serialize(); }) | to_vector;
  const auto totalSize = static_cast<uint32_t>(
      accumulate(rawBlocks | views::transform([](const auto &rawBlock) { return rawBlock.size(); }), 0));
  const auto rawSize = toBytes(totalSize);

  return views::concat(rawSize, rawBlocks | views::join) | to_vector;
}

SparseVoxelOctree SparseVoxelOctree::Deserialize(std::span<const std::byte> data) {
  const auto totalSize = fromBytes<uint32_t>(data.first(4));
  auto result = SparseVoxelOctree();
  auto offset = sizeof(totalSize);
  while (offset < totalSize + sizeof(totalSize)) {
    const auto blockSpan = data.subspan(offset);
    const auto blockSize = fromBytes<uint32_t>(blockSpan.first(4));
    result.blocks.emplace_back(Block::Deserialize(blockSpan));
    const auto oldOffset = offset;
    offset += sizeof(blockSize) + blockSize;
    assert(offset != oldOffset);
  }
  return result;
}
}// namespace pf::vox