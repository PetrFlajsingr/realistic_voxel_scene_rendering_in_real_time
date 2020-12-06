//
// Created by petr on 12/5/20.
//

#include "VulkanDebugCallbackImpl.h"
#include "logging/loggers.h"
#ifdef STACKTRACE_VULKAN_REPORT
#include <pf_common/exceptions/StackTraceException.h>
#endif
namespace pf {

bool VulkanDebugCallbackImpl::debugCallback(const pf::vulkan::DebugCallbackData &data,
                                            vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                            const vk::DebugUtilsMessageTypeFlagsEXT &) {
  auto log_level = spdlog::level::trace;
#ifdef STACKTRACE_VULKAN_REPORT
  auto stacktraceSrc = std::string();
#endif
  switch (severity) {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
      log_level = spdlog::level::trace;
      break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo: log_level = spdlog::level::info; break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: log_level = spdlog::level::warn; break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
#ifdef STACKTRACE_VULKAN_REPORT
      stacktraceSrc = traceToString(getTrace(13));
#endif
      log_level = spdlog::level::err;
      break;
  }
  const auto messageToPrint = makeValidationMessageReadable(data.message, 125);
#ifdef STACKTRACE_VULKAN_REPORT
  if (log_level == spdlog::level::err) {
    logFmt(log_level, VK_TAG, "Validation layer: {} message id: {}, stacktrace:\n{}",
           messageToPrint, data.messageId, stacktraceSrc);
  } else {
    logFmt(log_level, VK_TAG, "Validation layer: {} message id: {}", messageToPrint,
           data.messageId);
  }
#else
  logFmt(log_level, VK_TAG, "Validation layer: {} message id: {}", messageToPrint, data.messageId);
#endif
  return false;
}
std::string VulkanDebugCallbackImpl::makeValidationMessageReadable(std::string message,
                                                                   uint maxLineLength) {
  for (std::size_t i = maxLineLength; i < message.size(); i += maxLineLength) {
    message.insert(message.begin() + i, '\n');
  }
  return message;
}
}// namespace pf