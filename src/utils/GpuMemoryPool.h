//
// Created by petr on 5/30/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UTILS_GPUMEMORYPOOL_H
#define REALISTIC_VOXEL_RENDERING_SRC_UTILS_GPUMEMORYPOOL_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <pf_glfw_vulkan/vulkan/types/Buffer.h>
#include <pf_glfw_vulkan/vulkan/types/LogicalDevice.h>
#include <ranges>
#include <set>
#include <tl/expected.hpp>
#include <utility>
#include <vector>

#include <logging/loggers.h>

namespace pf::vulkan {

// TODO: alignment
template<std::size_t Align = 1>
class BufferMemoryPool : public VulkanObject,
                         public PtrConstructible<BufferMemoryPool<Align>>,
                         public std::enable_shared_from_this<BufferMemoryPool<Align>> {
 public:
  constexpr static auto Alignment = Align;
  using BlockId = std::size_t;
  [[nodiscard]] std::string info() const override { return fmt::format("Memory on buffer: {}", buffer->info()); }

  /**
   * Returns memory on destruction.
   */
  struct Block : public VulkanObject {
   public:
    friend class BufferMemoryPool;
    Block(BlockId id, vk::DeviceSize offset, vk::DeviceSize size, std::shared_ptr<BufferMemoryPool> parent)
        : id(id), offset_(offset), size(size), owner(std::move(parent)) {
      //logd(MAIN_TAG, "Allocated block: {}", info());
    }
    Block(const Block &) = delete;
    Block &operator=(const Block &) = delete;
    Block(Block &&other) noexcept : id(other.id), offset_(other.offset_), size(other.size), owner(other.owner) {
      other.valid = false;
    }

    Block &operator=(Block &&other) noexcept {
      other.valid = false;
      id = other.id;
      offset_ = other.offset_;
      size = other.size;
      owner = other.owner;
      return *this;
    }

    ~Block() override {
      if (valid) { owner->returnMemory(*this); }
    }

    [[nodiscard]] BufferMapping mapping(vk::DeviceSize offset = 0) {
      assert(valid);
      return BufferMapping(owner->getBuffer(), offset_ + offset + owner->getOffsetInBuffer(), getSize() - offset);
    }
    [[nodiscard]] BufferMapping mapping(vk::DeviceSize offset, vk::DeviceSize range) {
      assert(valid);
      return BufferMapping(owner->getBuffer(), offset_ + offset + owner->getOffsetInBuffer(), range);
    }

    [[nodiscard]] vk::DeviceSize getSize() const { return size; }
    [[nodiscard]] BlockId getId() const { return id; }
    [[nodiscard]] vk::DeviceSize getOffset() const { return offset_; }

    [[nodiscard]] std::string info() const override {
      if (!valid) { return "Invalid memory block."; }
      return fmt::format("Memory block from pool {}. Offset: {}, size: {}.", owner->info(), offset_, size);
    }

   private:
    BlockId id;
    vk::DeviceSize offset_;
    vk::DeviceSize size;
    std::shared_ptr<BufferMemoryPool> owner;
    mutable bool valid = true;
  };

  explicit BufferMemoryPool(std::shared_ptr<Buffer> b, vk::DeviceSize bufferOffset = 0,
                            std::optional<vk::DeviceSize> memSize = std::nullopt)
      : buffer(std::move(b)), offsetInBuffer(bufferOffset),
        allocatedMemorySize(memSize.value_or(buffer->getSize() - bufferOffset)) {
    availableMemory.emplace(0, allocatedMemorySize);
  }

  [[nodiscard]] tl::expected<Block, std::string> leaseMemory(vk::DeviceSize size) {
    assert(size != 0);
    size += size % Alignment;
    const Chunk *chunkToUse = nullptr;
    for (auto &chunk : availableMemory) {
      if (size <= chunk.size) {
        chunkToUse = &chunk;
        break;
      }
    }
    if (chunkToUse == nullptr) { return tl::make_unexpected("No chunk with enough memory available."); }
    auto block = Block{Id++, chunkToUse->offset, size,
                       std::enable_shared_from_this<BufferMemoryPool<Align>>::shared_from_this()};
    auto newChunk = Chunk{chunkToUse->offset + size, chunkToUse->size - size};
    availableMemory.erase(*chunkToUse);
    if (newChunk.size > 0) { availableMemory.emplace(newChunk); }
    return block;
  }

  void returnMemory(const Block &block) {
    if (!block.valid) { return; }
    //logd(MAIN_TAG, "Returning block: {}", block.info());
    block.valid = false;
    Chunk chunkToAdd{block.getOffset(), block.getSize()};
    const auto followingChunkOffset = block.getOffset() + block.getSize();
    std::vector<Chunk> chunksToRemove{};
    for (auto &chunk : availableMemory) {
      if (chunk.offset > followingChunkOffset) { break; }
      if (chunk.offset + chunk.size == block.getOffset()) {
        chunksToRemove.emplace_back(chunk);
        chunkToAdd.offset -= chunk.size;
        chunkToAdd.size += chunk.size;
      } else if (chunk.offset == followingChunkOffset) {
        chunksToRemove.emplace_back(chunk);
        chunkToAdd.size += chunk.size;
      }
    }
    std::ranges::for_each(chunksToRemove, [this](auto &chunkToRemove) { availableMemory.erase(chunkToRemove); });
    availableMemory.emplace(chunkToAdd);
  }

  [[nodiscard]] const std::shared_ptr<Buffer> &getBuffer() const { return buffer; }
  [[nodiscard]] vk::DeviceSize getOffsetInBuffer() const { return offsetInBuffer; }

 private:
  struct Chunk {
    vk::DeviceSize offset;
    vk::DeviceSize size;
    inline bool operator<(const Chunk &rhs) const { return offset < rhs.offset; }
    inline bool operator>(const Chunk &rhs) const { return rhs < *this; }
    inline bool operator<=(const Chunk &rhs) const { return !(rhs < *this); }
    inline bool operator>=(const Chunk &rhs) const { return !(*this < rhs); }
    inline bool operator==(const Chunk &rhs) const { return offset == rhs.offset; }
    inline bool operator!=(const Chunk &rhs) const { return !(rhs == *this); }
  };

  std::shared_ptr<Buffer> buffer;
  vk::DeviceSize offsetInBuffer;
  std::set<Chunk> availableMemory;
  vk::DeviceSize allocatedMemorySize;
  static inline std::size_t Id = 0;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_RENDERING_SRC_UTILS_GPUMEMORYPOOL_H
