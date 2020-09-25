//
// Created by petr on 9/25/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_DEFAULT_DEVICE_SUITABILITY_SCORER_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_DEFAULT_DEVICE_SUITABILITY_SCORER_H
#include <functional>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
using device_suitability_score = std::size_t;
using device_suitability_score_result = std::optional<device_suitability_score>;
using device_suitability_scorer_fnc =
    std::function<device_suitability_score_result(const vk::PhysicalDevice &)>;
template<typename T>
concept device_suitability_scorer = std::invocable<T, const vk::PhysicalDevice &> &&std::same_as<
    device_suitability_score_result, std::invoke_result_t<T, const vk::PhysicalDevice &>>;

struct default_device_suitability_scorer {
 public:
  using feature_score_fnc =
      std::function<device_suitability_score_result(const vk::PhysicalDeviceFeatures &)>;

  explicit default_device_suitability_scorer(
      std::unordered_set<std::string> required_extensions,
      std::unordered_map<std::string, device_suitability_score> optional_extensions,
      feature_score_fnc feature_check);

  device_suitability_score_result operator()(const vk::PhysicalDevice &device);

 private:
  std::unordered_set<std::string> required_extensions;
  std::unordered_map<std::string, device_suitability_score> optional_extensions;
  feature_score_fnc feature_check;
};
static_assert(device_suitability_scorer<default_device_suitability_scorer>);
}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_DEFAULT_DEVICE_SUITABILITY_SCORER_H
