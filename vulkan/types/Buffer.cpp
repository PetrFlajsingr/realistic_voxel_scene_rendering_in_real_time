//
// Created by petr on 10/19/20.
//

#include "Buffer.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"

namespace pf::vulkan {

BufferMapping::BufferMapping(std::shared_ptr<Buffer> buff, vk::DeviceSize start,
                             vk::DeviceSize count)
    : buffer(std::move(buff)), offset(start), range(count) {
  dataPtr = buffer->getLogicalDevice()->mapMemory(buffer->getMemory(), offset, range, {});
}

BufferMapping::~BufferMapping() { buffer->getLogicalDevice()->unmapMemory(buffer->getMemory()); }
std::string BufferMapping::info() const {
  return fmt::format("Buffer mapping to '{}' with offset: {}, range: {}", buffer->info(), offset,
                     range);
}
vk::DeviceSize BufferMapping::getSize() const {
  return range - offset;
}
void *BufferMapping::rawData() {
  return dataPtr;
}

Buffer::Buffer(std::shared_ptr<LogicalDevice> device, BufferConfig &&config,
               bool allocateImmediately)
    : logicalDevice(std::move(device)), size(config.size), usageFlags(config.usageFlags),
      sharingMode(config.sharingMode) {
  auto createInfo = vk::BufferCreateInfo();
  createInfo.size = size;
  createInfo.usage = usageFlags;
  createInfo.sharingMode = sharingMode;
  createInfo.setQueueFamilyIndices(config.queueFamilyIndices);
  vkBuffer = logicalDevice->getVkLogicalDevice().createBufferUnique(createInfo);
  if (allocateImmediately) { allocate(); }
}

void Buffer::allocate() {
  const auto bufferMemRequirements =
      logicalDevice->getVkLogicalDevice().getBufferMemoryRequirements(*vkBuffer);

  auto allocationInfo = vk::MemoryAllocateInfo();
  allocationInfo.allocationSize = bufferMemRequirements.size;
  allocationInfo.memoryTypeIndex = findMemoryType(bufferMemRequirements.memoryTypeBits);
  vkMemory = logicalDevice->getVkLogicalDevice().allocateMemoryUnique(allocationInfo);
  logicalDevice->getVkLogicalDevice().bindBufferMemory(*vkBuffer, *vkMemory, 0);
  isAllocated_ = true;
}

vk::DeviceSize Buffer::getSize() const { return size; }

const vk::BufferUsageFlags &Buffer::getUsageFlags() const { return usageFlags; }

vk::SharingMode Buffer::getSharingMode() const { return sharingMode; }

const vk::Buffer &Buffer::getVkBuffer() const { return *vkBuffer; }

const vk::Buffer &Buffer::operator*() const { return *vkBuffer; }
vk::Buffer const *Buffer::operator->() const { return &*vkBuffer; }
bool Buffer::isAllocated() const { return isAllocated_; }

uint32_t Buffer::findMemoryType(uint32_t memoryTypeBits) {
  const auto properties =
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
  auto memoryProperties = logicalDevice->getPhysicalDevice()->getMemoryProperties();
  for (const auto &[idx, prop] : ranges::views::enumerate(memoryProperties.memoryTypes)) {
    if ((memoryTypeBits & (1u << idx)) && (prop.propertyFlags & properties) == properties) {
      return idx;
    }
  }
  throw VulkanException("Could not find fitting memory type");
}
const LogicalDevice &Buffer::getLogicalDevice() const { return *logicalDevice; }

std::shared_ptr<BufferView> Buffer::createView(BufferViewConfig &&config) {
  return BufferView::CreateShared(shared_from_this(), std::move(config));
}
const vk::DeviceMemory &Buffer::getMemory() const { return *vkMemory; }
BufferMapping Buffer::mapping(vk::DeviceSize offset, vk::DeviceSize range) {
  return BufferMapping(shared_from_this(), offset, range);
}
std::shared_ptr<BufferMapping> Buffer::mappingShared(vk::DeviceSize offset, vk::DeviceSize range) {
  return BufferMapping::CreateShared(shared_from_this(), offset, range);
}
std::string Buffer::info() const { return fmt::format("Buffer object of size {}", size); }

}// namespace pf::vulkan