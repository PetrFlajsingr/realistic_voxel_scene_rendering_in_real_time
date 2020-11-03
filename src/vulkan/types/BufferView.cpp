//
// Created by petr on 10/19/20.
//

#include "BufferView.h"
#include "../VulkanException.h"
#include "Buffer.h"
#include "LogicalDevice.h"

namespace pf::vulkan {

BufferView::BufferView(std::shared_ptr<Buffer> buff, BufferViewConfig &&config)
    : buffer(std::move(buff)), format(config.format), offset(config.offset), range(config.range) {
  if (config.offset >= buffer->getSize()) {
    throw VulkanException("Invalid Buffer view size, offset bigger than buffer size");
  }
  if (config.offset + config.range > buffer->getSize()) {
    throw VulkanException("Invalid Buffer view size, offset + range bigger than buffer size");
  }
  auto createInfo = vk::BufferViewCreateInfo();
  createInfo.flags = config.createFlags;
  createInfo.format = config.format;
  createInfo.offset = config.offset;
  createInfo.range = config.range;
  createInfo.buffer = **buffer;
  vkBufferView = buffer->getLogicalDevice()->createBufferViewUnique(createInfo);
}
const Buffer &BufferView::getBuffer() const { return *buffer; }

vk::Format BufferView::getFormat() const { return format; }

vk::DeviceSize BufferView::getOffset() const { return offset; }

vk::DeviceSize BufferView::getRange() const { return range; }

const vk::BufferView &BufferView::getVkBufferView() const { return *vkBufferView; }

std::string BufferView::info() const {
  return fmt::format("Buffer view to '{}', format: {}, offset: {}, range: {}", buffer->info(),
                     magic_enum::enum_name(format), offset, range);
}

}// namespace pf::vulkan