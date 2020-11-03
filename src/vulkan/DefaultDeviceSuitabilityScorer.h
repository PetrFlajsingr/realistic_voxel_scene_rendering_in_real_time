//
// Created by petr on 9/25/20.
//

#ifndef VOXEL_RENDER_DEFAULTDEVICESUITABILITYSCORER_H
#define VOXEL_RENDER_DEFAULTDEVICESUITABILITYSCORER_H

#include <functional>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
using DeviceSuitabilityScore = std::size_t;
using DeviceSuitabilityScoreResult = std::optional<DeviceSuitabilityScore>;
using DeviceSuitabilityScorerFnc =
    std::function<DeviceSuitabilityScoreResult(const vk::PhysicalDevice &)>;
template<typename T>
concept DeviceSuitabilityScorer = std::invocable<T, const vk::PhysicalDevice &> &&
    std::same_as<DeviceSuitabilityScoreResult, std::invoke_result_t<T, const vk::PhysicalDevice &>>;

struct DefaultDeviceSuitabilityScorer {
 public:
  using FeatureScoreFnc =
      std::function<DeviceSuitabilityScoreResult(const vk::PhysicalDeviceFeatures &)>;

  explicit DefaultDeviceSuitabilityScorer(
      std::unordered_set<std::string> requiredExtensions,
      std::unordered_map<std::string, DeviceSuitabilityScore> optionalExtensions,
      FeatureScoreFnc featureChecker);

  DeviceSuitabilityScoreResult operator()(const vk::PhysicalDevice &device);

 private:
  std::unordered_set<std::string> reqExtensions;
  std::unordered_map<std::string, DeviceSuitabilityScore> optExtensions;
  FeatureScoreFnc featureCheck;
};
static_assert(DeviceSuitabilityScorer<DefaultDeviceSuitabilityScorer>);
}// namespace pf::vulkan

#endif//VOXEL_RENDER_DEFAULTDEVICESUITABILITYSCORER_H
