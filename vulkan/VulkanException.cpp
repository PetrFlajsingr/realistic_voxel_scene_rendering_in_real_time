//
// Created by petr on 9/25/20.
//

#include "VulkanException.h"
#include "fmt/format.h"

namespace pf::vulkan {
VulkanException::VulkanException(const std::string_view &message) : StackTraceException(message) {}
}// namespace pf::vulkan