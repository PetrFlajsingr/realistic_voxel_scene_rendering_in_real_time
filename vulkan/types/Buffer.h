//
// Created by petr on 10/19/20.
//

#ifndef VOXEL_RENDER_BUFFER_H
#define VOXEL_RENDER_BUFFER_H

#include "../concepts/PtrConstructible.h"
#include "BufferView.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>
#include <span>

namespace pf::vulkan {

struct BufferConfig {
  vk::DeviceSize size;
  vk::BufferUsageFlags usageFlags;
  vk::SharingMode sharingMode;
  std::vector<uint32_t> queueFamilyIndices;
};

class BufferMapping : public VulkanObject, public PtrConstructible<BufferMapping> {
 public:
  BufferMapping(std::shared_ptr<Buffer> buff, vk::DeviceSize start, vk::DeviceSize count);
  ~BufferMapping() override;

  [[nodiscard]] void *rawData();

  template<typename T>
  std::span<T> data() {
    return data<T>(0);
  }
  template<typename T>
  std::span<T> data(vk::DeviceSize start) {
    const auto count = getTypedSize<T>() - start;
    return data<T>(start, count);
  }
  template<typename T>
  std::span<T> data(vk::DeviceSize start, vk::DeviceSize count) {
    const auto typedSize = getTypedSize<T>();
    assert(start < typedSize);
    assert(start + count < typedSize);
    return std::span(reinterpret_cast<T*>(dataPtr) + start, count);
  }

  [[nodiscard]] vk::DeviceSize getSize() const;

  template <typename T>
  [[nodiscard]] vk::DeviceSize getTypedSize() const {
    return getSize() / sizeof(T);
  }

  [[nodiscard]] std::string info() const override;

 private:
  std::shared_ptr<Buffer> buffer;
  vk::DeviceSize offset;
  vk::DeviceSize range;

  void *dataPtr = nullptr;
};

class Buffer : public VulkanObject,
               public PtrConstructible<Buffer>,
               public std::enable_shared_from_this<Buffer> {
 public:
  Buffer(std::shared_ptr<LogicalDevice> device, BufferConfig &&config,
         bool allocateImmediately = false);

  void allocate();

  [[nodiscard]] const LogicalDevice &getLogicalDevice() const;
  [[nodiscard]] vk::DeviceSize getSize() const;
  [[nodiscard]] const vk::BufferUsageFlags &getUsageFlags() const;
  [[nodiscard]] vk::SharingMode getSharingMode() const;
  [[nodiscard]] const vk::Buffer &getVkBuffer() const;
  [[nodiscard]] const vk::DeviceMemory &getMemory() const;

  const vk::Buffer &operator*() const;
  vk::Buffer const *operator->() const;

  [[nodiscard]] bool isAllocated() const;

  [[nodiscard]] std::shared_ptr<BufferView> createView(BufferViewConfig &&config);
  [[nodiscard]] BufferMapping mapping(vk::DeviceSize offset, vk::DeviceSize range);
  [[nodiscard]] std::shared_ptr<BufferMapping> mappingShared(vk::DeviceSize offset,
                                                             vk::DeviceSize range);

  [[nodiscard]] std::string info() const override;

 private:
  uint32_t findMemoryType(uint32_t memoryTypeBits);

  std::shared_ptr<LogicalDevice> logicalDevice;
  vk::DeviceSize size;
  vk::BufferUsageFlags usageFlags;
  vk::SharingMode sharingMode;

  bool isAllocated_ = false;

  vk::UniqueDeviceMemory vkMemory;
  vk::UniqueBuffer vkBuffer;
};

}// namespace pf::vulkan

#endif//VOXEL_RENDER_BUFFER_H
