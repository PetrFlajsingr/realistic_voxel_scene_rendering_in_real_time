//
// Created by petr on 9/25/20.
//

#include "VulkanException.h"
#include "fmt/format.h"

VulkanException::VulkanException(const std::string_view &message) : StackTraceException(message) {}
