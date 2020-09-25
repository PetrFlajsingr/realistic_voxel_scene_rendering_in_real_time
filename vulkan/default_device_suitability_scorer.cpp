//
// Created by petr on 9/25/20.
//

#include "default_device_suitability_scorer.h"

pf::vulkan::default_device_suitability_scorer::default_device_suitability_scorer(
    std::unordered_set<std::string> required_extensions,
    std::unordered_map<std::string, device_suitability_score> optional_extensions,
    pf::vulkan::default_device_suitability_scorer::feature_score_fnc feature_check)
    : required_extensions(std::move(required_extensions)),
      optional_extensions(std::move(optional_extensions)), feature_check(std::move(feature_check)) {
}

pf::vulkan::device_suitability_score_result
pf::vulkan::default_device_suitability_scorer::operator()(const vk::PhysicalDevice &device) {
  auto not_found_required_ext = required_extensions;
  auto score = std::size_t(0);
  for (const auto &extension : device.enumerateDeviceExtensionProperties()) {
    not_found_required_ext.erase(extension.extensionName);
    if (optional_extensions.contains(extension.extensionName)) {
      score += optional_extensions[extension.extensionName];
    }
  }
  if (!not_found_required_ext.empty()) { return std::nullopt; }
  if (const auto feature_score = feature_check(device.getFeatures()); feature_score.has_value()) {
    score += feature_score.value();
  } else {
    return std::nullopt;
  }
  return score;
}
