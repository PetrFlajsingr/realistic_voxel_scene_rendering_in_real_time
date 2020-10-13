//
// Created by petr on 9/25/20.
//

#include "DefaultDeviceSuitabilityScorer.h"

pf::vulkan::DefaultDeviceSuitabilityScorer::DefaultDeviceSuitabilityScorer(
    std::unordered_set<std::string> requiredExtensions,
    std::unordered_map<std::string, DeviceSuitabilityScore> optionalExtensions,
    pf::vulkan::DefaultDeviceSuitabilityScorer::FeatureScoreFnc featureCheck)
    : requiredExtensions(std::move(requiredExtensions)),
      optionalExtensions(std::move(optionalExtensions)), featureCheck(std::move(featureCheck)) {}

pf::vulkan::DeviceSuitabilityScoreResult
pf::vulkan::DefaultDeviceSuitabilityScorer::operator()(const vk::PhysicalDevice &device) {
  auto notFoundRequiredExt = requiredExtensions;
  auto score = std::size_t(0);
  for (const auto &extension : device.enumerateDeviceExtensionProperties()) {
    notFoundRequiredExt.erase(extension.extensionName);
    if (optionalExtensions.contains(extension.extensionName)) {
      score += optionalExtensions[extension.extensionName];
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
