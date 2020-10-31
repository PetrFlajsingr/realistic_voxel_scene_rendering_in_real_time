//
// Created by petr on 9/26/20.
//

#ifndef VOXEL_RENDER_SWAPCHAIN_H
#define VOXEL_RENDER_SWAPCHAIN_H

#include "../concepts/PtrConstructible.h"
#include "../concepts/Window.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <set>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct SwapChainConfig {
  std::set<vk::SurfaceFormatKHR> formats;
  std::set<vk::PresentModeKHR> presentModes;
  ui::Resolution resolution;
  vk::ImageUsageFlags imageUsage;
  std::unordered_set<uint32_t> sharingQueues;
  uint32_t imageArrayLayers;
  bool clipped;
  std::optional<vk::SwapchainKHR> oldSwapChain;
  vk::CompositeAlphaFlagBitsKHR compositeAlpha;
};

struct PresentConfig {
  std::vector<std::reference_wrapper<Semaphore>> waitSemaphores;
  vk::Queue presentQueue;
};

class SwapChain : public VulkanObject,
                  public PtrConstructible<SwapChain>,
                  public std::enable_shared_from_this<SwapChain> {
 public:
  explicit SwapChain(std::shared_ptr<Surface> surf, std::shared_ptr<LogicalDevice> device,
                     pf::vulkan::SwapChainConfig &&config);

  void init();

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

  [[nodiscard]] vk::Format getFormat() const;
  [[nodiscard]] vk::ColorSpaceKHR getColorSpace() const;
  [[nodiscard]] const vk::Extent2D &getExtent() const;
  [[nodiscard]] LogicalDevice &getLogicalDevice();
  [[nodiscard]] Surface &getSurface();

  const vk::SwapchainKHR &operator*() const;
  vk::SwapchainKHR const *operator->() const;

  [[nodiscard]] std::string info() const override;

  [[nodiscard]] const std::vector<std::shared_ptr<ImageRef>> &getImages() const;
  [[nodiscard]] const std::vector<std::shared_ptr<ImageView>> &getImageViews() const;
  [[nodiscard]] const std::vector<std::shared_ptr<FrameBuffer>> &getFrameBuffers() const;

  void swap();

  void frameDone();

  [[nodiscard]] Semaphore &getCurrentSemaphore() const;
  [[nodiscard]] Fence &getCurrentFence() const;

  void checkRebuild();

  void present(PresentConfig &&config) const;


  void addRebuildListener(std::invocable auto &&f) {
    rebuildListeners.emplace_back(f);
  }

  [[nodiscard]] std::size_t getCurrentImageIndex() const;
  [[nodiscard]] std::size_t getCurrentFrameIndex() const;

 private:
  static vk::PresentModeKHR
  selectPresentMode(const std::set<vk::PresentModeKHR> &present_modes,
                    const std::vector<vk::PresentModeKHR> &surface_present_modes);

  static vk::SurfaceFormatKHR
  selectSurfaceFormat(const std::set<vk::SurfaceFormatKHR> &formats,
                      const std::vector<vk::SurfaceFormatKHR> &surfaceFormats);

  static vk::Extent2D selectExtent(const ui::Resolution &resolution,
                                   const vk::SurfaceCapabilitiesKHR &surfaceCapabilities);

  static vk::UniqueSwapchainKHR
  createSwapChainHandle(Surface &surface, LogicalDevice &logicalDevice,
                        const vk::SurfaceCapabilitiesKHR &surfaceCapabilities,
                        const pf::vulkan::SwapChainConfig &config,
                        vk::SurfaceFormatKHR surfaceFormat, vk::Extent2D extent,
                        vk::PresentModeKHR presentMode);

  void initImagesAndImageViews();
  void initFrameBuffers();

  std::optional<ui::Resolution> windowResolutionCheck();

  void rebuildSwapChain(ui::Resolution resolution);

  vk::UniqueSwapchainKHR vkSwapChain;
  vk::Format format;
  vk::ColorSpaceKHR colorSpace;
  vk::Extent2D extent;
  std::shared_ptr<LogicalDevice> logicalDevice;
  std::shared_ptr<Surface> surface;

  std::set<vk::SurfaceFormatKHR> formats;
  std::set<vk::PresentModeKHR> presentModes;
  vk::ImageUsageFlags imageUsage;
  std::unordered_set<uint32_t> sharingQueues;
  uint32_t imageArrayLayers;
  bool clipped;
  vk::CompositeAlphaFlagBitsKHR compositeAlpha;

  std::vector<std::shared_ptr<ImageRef>> images;
  std::vector<std::shared_ptr<ImageView>> imageViews;
  std::vector<std::shared_ptr<FrameBuffer>> frameBuffers;

  std::vector<std::shared_ptr<vulkan::Semaphore>> imageSemaphores;
  std::vector<std::shared_ptr<vulkan::Fence>> imageFences;
  std::vector<std::shared_ptr<vulkan::Fence>> usedImageFences;

  std::vector<std::function<void()>> rebuildListeners;

  std::size_t frameIdx = 0;
  std::size_t imageIdx;
};
}// namespace pf::vulkan
#endif//VOXEL_RENDER_SWAPCHAIN_H
