//
// Created by petr on 9/25/20.
//

#include "DefaultDeviceSuitabilityScorer.h"

namespace pf::vulkan {
DefaultDeviceSuitabilityScorer::DefaultDeviceSuitabilityScorer(
    std::unordered_set<std::string> requiredExtensions,
    std::unordered_map<std::string, DeviceSuitabilityScore> optionalExtensions,
    DefaultDeviceSuitabilityScorer::FeatureScoreFnc featureChecker)
    : reqExtensions(std::move(requiredExtensions)), optExtensions(std::move(optionalExtensions)),
      featureCheck(std::move(featureChecker)) {}

DeviceSuitabilityScoreResult
DefaultDeviceSuitabilityScorer::operator()(const vk::PhysicalDevice &device) {
  auto notFoundRequiredExt = reqExtensions;
  auto score = std::size_t(0);
  for (const auto &extension : device.enumerateDeviceExtensionProperties()) {
    notFoundRequiredExt.erase(extension.extensionName);
    if (optExtensions.contains(extension.extensionName)) {
      score += optExtensions[extension.extensionName];
    }
  }
  if (!notFoundRequiredExt.empty()) { return std::nullopt; }
  if (const auto featureScore = featureCheck(device.getFeatures()); featureScore.has_value()) {
    score += featureScore.value();
  } else {
    return std::nullopt;
  }
  return score;
}
}// namespace pf::vulkan
