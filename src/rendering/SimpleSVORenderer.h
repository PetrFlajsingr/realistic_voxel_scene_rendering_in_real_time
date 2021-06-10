//
// Created by petr on 1/5/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_SIMPLESVORENDERER_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_SIMPLESVORENDERER_H

#include "VulkanDebugCallbackImpl.h"
#include "logging/loggers.h"
#include "ui/ImGuiGlfwVulkan.h"
#include "utils/common_enums.h"
#include <RunInfo.h>
#include <chaiscript/chaiscript.hpp>
#include <pf_glfw_vulkan/lib_config.h>
#include <pf_glfw_vulkan/ui/Window.h>
#include <pf_glfw_vulkan/ui/events/common.h>
#include <pf_glfw_vulkan/vulkan/types.h>
#include <pf_imgui/elements/ProgressBar.h>
#include <range/v3/view/map.hpp>
#include <thread>
//#include <threading/ThreadPool.h>
#include <pf_common/parallel/ThreadPool.h>
#include <pf_glfw_vulkan/vulkan/types/BufferMemoryPool.h>
#include <toml++/toml.h>
#include <ui/SimpleSVORenderer_UI.h>
#include <utility>
#include <utils/Camera.h>
#include <utils/FPSCounter.h>
#include <voxel/AABB_BVH.h>
#include <voxel/GPUModelManager.h>
#include <voxel/SparseVoxelOctree.h>

namespace pf {
/*
enum class UniformLayout {
  std140
};// update alignmentForLayoutInBytes, sizeForLayoutInBytes and offsetForLayoutInBytes when adding new values

template<typename T>
consteval std::size_t alignmentForLayoutInBytes(UniformLayout layout) {
  if (layout == UniformLayout::std140) {
    if (OneOf<T, int, int32_t, unsigned int, uint32_t, float, bool>) { return 4; }
    if (OneOf<T, double, glm::ivec2, glm::uvec2, glm::bvec2, glm::vec2>) { return 8; }
    if (OneOf<T, glm::dvec2, glm::ivec3, glm::uvec3, glm::bvec3, glm::vec3, glm::ivec4, glm::uvec4, glm::bvec4,
              glm::vec4>) {
      return 16;
    }
    if (OneOf<T, glm::dvec3, glm::dvec4>) { return 32; }
    if (OneOf<T, glm::mat3, glm::mat4>) { return 16; }
  }
  return 0;// unknown
}

template<typename T>
consteval std::size_t sizeForLayoutInBytes(UniformLayout layout) {
  if (layout == UniformLayout::std140) {
    if (OneOf<T, int, int32_t, unsigned int, uint32_t, float, bool>) { return 4; }
    if (OneOf<T, double, glm::ivec2, glm::uvec2, glm::bvec2, glm::vec2>) { return 8; }
    if (OneOf<T, glm::ivec3, glm::uvec3, glm::bvec3, glm::vec3>) { return 12; }
    if (OneOf<T, glm::dvec2, glm::ivec4, glm::uvec4, glm::bvec4, glm::vec4>) { return 16; }
    if (OneOf<T, glm::dvec3>) { return 24; }
    if (OneOf<T, glm::dvec4>) { return 32; }
    if (OneOf<T, glm::mat3>) { return 3 * 16; }
    if (OneOf<T, glm::mat4>) { return 4 * 16; }
  }
}

template<typename T>
consteval std::size_t offsetForLayoutInBytes(UniformLayout layout) {
  if (layout == UniformLayout::std140) {
    if (OneOf<T, int, int32_t, unsigned int, uint32_t, float, bool>) { return 4; }
    if (OneOf<T, double, glm::ivec2, glm::uvec2, glm::bvec2, glm::vec2>) { return 8; }
    if (OneOf<T, glm::ivec3, glm::uvec3, glm::bvec3, glm::vec3, glm::dvec2, glm::ivec4, glm::uvec4, glm::bvec4,
              glm::vec4>) {
      return 16;
    }
    if (OneOf<T, glm::dvec3, glm::dvec4>) { return 32; }
    if (OneOf<T, glm::mat3, glm::mat4>) { return 4 * 16; }
  }
}

template<typename Tuple, UniformLayout Layout, std::size_t... Index>
requires (sizeof...(Index) > 0)
consteval std::size_t Count(std::index_sequence<Index...> const &) {
  return (offsetForLayoutInBytes<std::tuple_element_t<Index, Tuple>>(Layout) + ...);
}
template<typename Tuple, UniformLayout Layout, std::size_t... Index>
requires (sizeof...(Index) == 0)
consteval std::size_t Count(std::index_sequence<Index...> const &) {
  return 0;
}

template<std::size_t N, UniformLayout Layout, typename... Args>
consteval std::size_t countOffsetForTypesForLayout() {
  using UtilTuple = std::tuple<Args...>;
  return Count<UtilTuple, Layout>(std::make_index_sequence<N>());
}

template<typename T>
concept GlslType = OneOf<T, int, int32_t, unsigned int, uint32_t, float, bool, double, glm::ivec2, glm::uvec2,
                         glm::bvec2, glm::vec2, glm::ivec3, glm::uvec3, glm::bvec3, glm::vec3, glm::dvec2, glm::ivec4,
                         glm::uvec4, glm::bvec4, glm::vec4, glm::dvec3, glm::dvec4, glm::mat3, glm::mat4>;

template<UniformLayout Layout, GlslType... Args>
class UniformAccessor {
  using UtilTuple = std::tuple<Args...>;

 public:
  explicit UniformAccessor(std::shared_ptr<vulkan::Buffer> buffer) : buffer(std::move(buffer)) {}
  template<std::size_t Index, typename T>
  requires(std::same_as<std::decay_t<T>, std::tuple_element_t<Index, UtilTuple>>)
      && (OneOf<std::decay_t<T>, Args...>) void set(T &&value) {
    constexpr auto offset = countOffsetForTypesForLayout<Index, Layout, Args...>();
    buffer->mapping().template setRawOffset(std::forward<T>(value), offset);
  }
  template<std::size_t Index>
  std::tuple_element_t<Index, UtilTuple> get() {
    constexpr auto offset = countOffsetForTypesForLayout<Index, Layout, Args...>();
    return *reinterpret_cast<std::tuple_element_t<Index, UtilTuple> *>(
        &buffer->mapping().template data<std::byte>()[offset]);
  }

 private:
  std::shared_ptr<vulkan::Buffer> buffer;
};*/

class SimpleSVORenderer : public VulkanDebugCallbackImpl {
 public:
  explicit SimpleSVORenderer(toml::table &tomlConfig);
  SimpleSVORenderer(const SimpleSVORenderer &) = delete;
  SimpleSVORenderer &operator=(const SimpleSVORenderer &) = delete;
  SimpleSVORenderer(SimpleSVORenderer &&) = default;
  SimpleSVORenderer &operator=(SimpleSVORenderer &&) = default;
  virtual ~SimpleSVORenderer();

  void init(const std::shared_ptr<ui::Window> &win);

  void render();
  void stop();

 private:
  static std::unordered_set<std::string> getValidationLayers();

  void buildVulkanObjects();

  void createInstance();

  void createSurface();
  void createDevices();

  void createSwapchain();
  void createRenderTextures();
  void createDescriptorPool();
  void createPipeline();
  void createCommands();
  void recordCommands();
  void createFences();
  void createSemaphores();

  void initUI();

  void rebuildAndUploadBVH();

  std::vector<std::filesystem::path> loadModelFileNames(const std::filesystem::path &dir);
  std::tuple<ui::ig::ModalDialog &, ui::ig::ProgressBar<float> &, ui::ig::Text &> createLoadingDialog();

  void addActiveModelPopupMenu(ui::ig::Selectable &element, std::size_t itemId,
                               vox::GPUModelManager::ModelPtr modelPtr);
  std::function<void()> popupClickActiveModel(std::size_t modelId, vox::GPUModelManager::ModelPtr modelPtr);

  void duplicateModel(vox::GPUModelManager::ModelPtr original);
  void instantiateModel(vox::GPUModelManager::ModelPtr original);

  std::reference_wrapper<toml::table> config;
  Camera camera;

  std::shared_ptr<vulkan::DescriptorPool> vkDescPool;
  std::vector<vk::UniqueDescriptorSet> computeDescriptorSets;
  std::shared_ptr<vulkan::Instance> vkInstance;
  std::shared_ptr<vulkan::Surface> vkSurface;
  std::shared_ptr<vulkan::PhysicalDevice> vkDevice;
  std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice;
  std::shared_ptr<vulkan::SwapChain> vkSwapChain;
  std::shared_ptr<vulkan::CommandPool> vkCommandPool;
  std::shared_ptr<vulkan::CommandPool> vkGraphicsCommandPool;
  std::vector<std::shared_ptr<vulkan::CommandBuffer>> vkCommandBuffers;
  std::vector<std::shared_ptr<vulkan::CommandBuffer>> vkGraphicsCommandBuffers;

  std::shared_ptr<vulkan::Image> vkRenderImage;
  std::shared_ptr<vulkan::ImageView> vkRenderImageView;
  std::shared_ptr<vulkan::Image> vkIterImage;
  std::shared_ptr<vulkan::ImageView> vkIterImageView;
  std::shared_ptr<vulkan::TextureSampler> vkIterImageSampler;

  std::shared_ptr<vulkan::DescriptorSetLayout> vkComputeDescSetLayout;

  std::shared_ptr<vulkan::Buffer> cameraUniformBuffer;
  std::shared_ptr<vulkan::Buffer> lightUniformBuffer;
  std::shared_ptr<vulkan::Buffer> debugUniformBuffer;
  std::shared_ptr<vulkan::Buffer> svoBuffer;
  std::shared_ptr<vulkan::Buffer> modelInfoBuffer;
  std::shared_ptr<vulkan::Buffer> bvhBuffer;
  std::shared_ptr<vulkan::Semaphore> computeSemaphore;
  std::vector<std::shared_ptr<vulkan::Semaphore>> renderSemaphores;

  std::vector<std::shared_ptr<vulkan::Fence>> fences;
  std::shared_ptr<vulkan::ComputePipeline> vkComputePipeline;
  std::shared_ptr<vulkan::Fence> vkComputeFence;
  std::shared_ptr<vulkan::RenderPass> vkRenderPass;

  std::unique_ptr<SimpleSVORenderer_UI> ui;

  FPSCounter fpsCounter;

  std::unique_ptr<chaiscript::ChaiScript> chai = std::make_unique<chaiscript::ChaiScript>();

  std::vector<Subscription> subscriptions;

  std::function<void()> closeWindow;

  std::shared_ptr<ui::Window> window;

  std::pair<std::size_t, std::size_t> computeLocalSize{1, 1};

  std::unique_ptr<ThreadPool> threadpool = std::make_unique<ThreadPool>(4);

  std::shared_ptr<vulkan::BufferMemoryPool> svoMemoryPool;
  std::shared_ptr<vulkan::BufferMemoryPool> modelInfoMemoryPool;

  std::unique_ptr<vox::GPUModelManager> modelManager;
};

}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_SIMPLESVORENDERER_H
