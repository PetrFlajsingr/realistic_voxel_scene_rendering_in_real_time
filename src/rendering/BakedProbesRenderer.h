//
// Created by petr on 1/5/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_BAKEDPROBESRENDERER_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_BAKEDPROBESRENDERER_H

#include "VulkanDebugCallbackImpl.h"
#include "enums.h"
#include "light_field_probes/ProbeBakeRenderer.h"
#include "logging/loggers.h"
#include "utils/common_enums.h"
#include <chaiscript/chaiscript.hpp>
#include <pf_common/parallel/ThreadPool.h>
#include <pf_glfw_vulkan/lib_config.h>
#include <pf_glfw_vulkan/ui/Window.h>
#include <pf_glfw_vulkan/ui/events/common.h>
#include <pf_glfw_vulkan/vulkan/types.h>
#include <pf_glfw_vulkan/vulkan/types/BufferMemoryPool.h>
#include <pf_imgui/elements/ProgressBar.h>
#include <range/v3/view/map.hpp>
#include <thread>
#include <toml++/toml.h>
#include <ui/SVOUI.h>
#include <utility>
#include <utils/Camera.h>
#include <utils/FPSCounter.h>
#include <voxel/AABB_BVH.h>
#include <voxel/GPUModelManager.h>
#include <voxel/SparseVoxelOctree.h>

namespace pf {

class BakedProbesRenderer : public VulkanDebugCallbackImpl {
 public:
  explicit BakedProbesRenderer(toml::table &tomlConfig);
  BakedProbesRenderer(const BakedProbesRenderer &) = delete;
  BakedProbesRenderer &operator=(const BakedProbesRenderer &) = delete;
  BakedProbesRenderer(BakedProbesRenderer &&) = default;
  BakedProbesRenderer &operator=(BakedProbesRenderer &&) = default;
  virtual ~BakedProbesRenderer();

  void init(const std::shared_ptr<ui::Window> &win);

  void render();
  void stop();

 private:
  static std::unordered_set<std::string> getValidationLayers();

  void buildVulkanObjects();

  void createBuffers();

  void createInstance();

  void createSurface();
  void createDevices();

  void createSwapchain();
  void createTextures();
  void createDescriptorPool();
  void createPipeline();
  void createCommands();
  void recordCommands();
  void createFences();
  void createSemaphores();

  void initUI();

  void rebuildAndUploadBVH();

  std::vector<std::filesystem::path> loadModelFileNames(const std::filesystem::path &dir);

  void addActiveModelPopupMenu(ui::ig::Selectable &element, std::size_t itemId,
                               vox::GPUModelManager::ModelPtr modelPtr);
  std::function<void()> popupClickActiveModel(std::size_t modelId, vox::GPUModelManager::ModelPtr modelPtr);

  void duplicateModel(vox::GPUModelManager::ModelPtr original);
  void instantiateModel(vox::GPUModelManager::ModelPtr original);

  void convertAndSaveSVO(const std::filesystem::path &src, const std::filesystem::path &dir);

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
  std::shared_ptr<vulkan::Buffer> materialBuffer;
  std::shared_ptr<vulkan::Semaphore> computeSemaphore;
  std::vector<std::shared_ptr<vulkan::Semaphore>> renderSemaphores;
  std::shared_ptr<vulkan::Buffer> probePosBuffer;

  std::vector<std::shared_ptr<vulkan::Fence>> fences;
  std::shared_ptr<vulkan::ComputePipeline> vkComputePipeline;
  std::shared_ptr<vulkan::Fence> vkComputeFence;
  std::shared_ptr<vulkan::RenderPass> vkRenderPass;

  std::unique_ptr<SVOUI> ui;

  FPSCounter fpsCounter;

  std::unique_ptr<chaiscript::ChaiScript> chai = std::make_unique<chaiscript::ChaiScript>();

  std::vector<Subscription> subscriptions;

  std::function<void()> closeWindow;

  std::shared_ptr<ui::Window> window;

  std::pair<std::size_t, std::size_t> computeLocalSize{1, 1};

  std::unique_ptr<ThreadPool> threadpool = std::make_unique<ThreadPool>(4);

  std::shared_ptr<vulkan::BufferMemoryPool> svoMemoryPool;
  std::shared_ptr<vulkan::BufferMemoryPool> modelInfoMemoryPool;
  std::shared_ptr<vulkan::BufferMemoryPool> materialMemoryPool;

  std::unique_ptr<vox::GPUModelManager> modelManager;

  std::unique_ptr<lfp::ProbeBakeRenderer> probeRenderer;
};

}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_BAKEDPROBESRENDERER_H
