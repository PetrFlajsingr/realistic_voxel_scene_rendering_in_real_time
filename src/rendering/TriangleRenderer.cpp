//
// Created by petr on 9/26/20.
//

#include "TriangleRenderer.h"
#include "../utils/FlameGraphSampler.h"


vk::Format pf::TriangleRenderer::getDepthFormat() {
  const auto tiling = vk::ImageTiling::eLinear;
  const auto features = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
  return vk::Format::eD32Sfloat;
  for (auto format :
       {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}) {
    const auto properties = vkDevice->getPhysicalDevice().getFormatProperties(format);

    if (tiling == vk::ImageTiling::eLinear
        && (properties.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == vk::ImageTiling::eOptimal
               && (properties.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}
pf::TriangleRenderer::~TriangleRenderer() {
  if (vkLogicalDevice == nullptr) { return; }
  log(spdlog::level::info, APP_TAG, "Destroying renderer, waiting for device");
  vkLogicalDevice->wait();
  log(spdlog::level::info, APP_TAG, "Saving UI to config");
  imgui->updateConfig();
  config.get()["ui"].as_table()->insert_or_assign("imgui", imgui->getConfig());
}

void pf::TriangleRenderer::initUI() {
  using namespace std::string_literals;
  using namespace pf::ui::ig;

  auto styleSetter = [](ImGuiStyle &style) {
    ImVec4 *colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
    colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
    colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
    colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

    style.ChildRounding = 4.0f;
    style.FrameBorderSize = 1.0f;
    style.FrameRounding = 2.0f;
    style.GrabMinSize = 7.0f;
    style.PopupRounding = 2.0f;
    style.ScrollbarRounding = 12.0f;
    style.ScrollbarSize = 13.0f;
    style.TabBorderSize = 1.0f;
    style.TabRounding = 0.0f;
    style.WindowRounding = 4.0f;
  };
  imgui->setStyle(styleSetter);

  auto &debugWindow = imgui->createChild<Window>("debug_window", "Debug");

  auto &debugWindowTabs = debugWindow.createChild<TabBar>("debug_tabbar");
  auto &chaiTab = debugWindowTabs.addTab("chai_tab", "ChaiScript");
  auto &logTab = debugWindowTabs.addTab("log_tab", "Log");

  auto &logMemo = logTab.createChild<Memo>("log_output", "Log:", 100, true, true, 100);
  addLogListener([&logMemo = logMemo](auto record) { logMemo.addRecord(record); });
  auto &logErrMemo = logTab.createChild<Memo>("log_err_output", "Log err:", 100, true, true, 100);
  addLogListener([&logErrMemo = logErrMemo](auto record) { logErrMemo.addRecord(record); }, true);

  auto &chaiInputPanel = chaiTab.createChild<Panel>("chai_input_panel", "Input",
                                                    PanelLayout::Horizontal, ImVec2{0, 50});

  chaiInputPanel.createChild<Text>("chain_input_label", "Input:");
  auto &chaiInput =
      chaiInputPanel.createChild<InputText>("chai_input", "", "", TextInputType::MultiLine);
  auto &chai_output = chaiTab.createChild<Memo>("chai_output", "Output:", 100, true, true, 100);

  chai->add(chaiscript::fun(makeChaiPrintFnc<int>(chai_output)), "print");
  chai->add(chaiscript::fun(makeChaiPrintFnc<double>(chai_output)), "print");
  chai->add(chaiscript::fun(makeChaiPrintFnc<float>(chai_output)), "print");
  chai->add(chaiscript::fun(makeChaiPrintFnc<std::string>(chai_output)), "print");
  chaiInputPanel.createChild<Button>("chain_input_confirm", "Confirm")
      .addClickListener([&chaiInput = chaiInput, &chai_output = chai_output, this] {
        const auto input = chaiInput.getText();
        chai_output.addRecord(">>> "s + input);
        chaiInput.clear();
        try {
          chai->eval(input);
        } catch (const chaiscript::exception::eval_error &e) {
          chai_output.addRecord("<<< "s + e.pretty_print());
        }
      });

  chai->add(
      chaiscript::fun([](const std::string &str) { log(spdlog::level::debug, APP_TAG, str); }),
      "log");

  auto &infoWindow = imgui->createChild<Window>("infoWindow", "Stats");
  auto fpsPlot = &infoWindow.createChild<SimplePlot>("fps_plot", "Fps", PlotType::Histogram,
                                                     std::vector<float>{}, std::nullopt, 200, 0, 60,
                                                     ImVec2{0, 50});
  const auto fpsMsgTemplate = "FPS:\nCurrent: {0:0.2f}\nAverage: {0:0.2f}";
  auto fpsLabel = &infoWindow.createChild<Text>("fpsText", "FPS");
  infoWindow.createChild<Button>("fpsResetBtn", "Reset FPS").addClickListener([this] {
    fpsCounter.reset();
  });

  auto &cameraGroup = infoWindow.createChild<Group>("cameraGroup", "Camera");
  const auto cameraPosTemplate = "Position: {0:0.2f}x{1:0.2f}x{2:0.2f}";
  const auto cameraDirTemplate = "Direction: {0:0.2f}x{1:0.2f}x{2:0.2f}";
  auto cameraPosText = &cameraGroup.createChild<Text>("cameraPositionText", "");
  auto cameraDirText = &cameraGroup.createChild<Text>("cameraDirText", "");
  cameraGroup
      .createChild<Slider<float>>("cameraMoveSpeedSlider", "Movement speed", 0.1f, 50.f,
                                  camera.getMovementSpeed(), Persistent::Yes)
      .addValueListener([this](auto value) { camera.setMovementSpeed(value); });
  cameraGroup
      .createChild<Slider<float>>("cameraMouseSpeedSlider", "Mouse speed", 0.1f, 50.f,
                                  camera.getMouseSpeed(), Persistent::Yes)
      .addValueListener([this](auto value) { camera.setMouseSpeed(value); });
  cameraGroup
      .createChild<Slider<float>>("cameraFOVSlider", "Field of view", 1.f, 90.f,
                                  camera.getFieldOfView(), Persistent::Yes)
      .addValueListener([this](auto value) { camera.setFieldOfView(value); });

  fpsCounter.setOnNewFrame([this, cameraPosText, cameraDirText, cameraDirTemplate,
                            cameraPosTemplate, fpsPlot, fpsMsgTemplate,
                            fpsLabel](const FPSCounter &counter) {
    fpsPlot->addValue(counter.currentFPS());
    fpsLabel->setText(fmt::format(fpsMsgTemplate, counter.currentFPS(), counter.averageFPS()));
    const auto camPos = camera.getPosition();
    cameraPosText->setText(fmt::format(cameraPosTemplate, camPos.x, camPos.y, camPos.z));
    const auto camDir = camera.getFront();
    cameraDirText->setText(fmt::format(cameraDirTemplate, camDir.x, camDir.y, camDir.z));
  });
  statsFlameGraph = &infoWindow.createChild<FlameGraph>("statsFlameGraph", "Main loop");

  testTexture = vkLogicalDevice->createTexture(
      {.path = std::filesystem::path("/home/petr/Downloads/tex.png"),
       .channels = vulkan::TextureChannels::rgb_alpha,
       .mipLevels = 1,
       .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst},
      *vkCommandPool);

  auto sub = vk::ImageSubresourceRange();
  sub.layerCount = 1;
  sub.levelCount = 1;
  sub.baseMipLevel = 0;
  sub.baseArrayLayer = 0;
  sub.aspectMask = vk::ImageAspectFlagBits::eColor;
  testTextureView = testTexture->getImage().createImageView(vk::ColorSpaceKHR::eSrgbNonlinear,
                                                            vk::ImageViewType::e2D, sub);

  testTextureSampler = testTexture->createSampler(
      {.magFilter = vk::Filter::eNearest,
       .minFilter = vk::Filter::eNearest,
       .addressMode = {.u = vk::SamplerAddressMode::eClampToBorder,
                       .v = vk::SamplerAddressMode::eClampToBorder,
                       .w = vk::SamplerAddressMode::eClampToBorder},
       .maxAnisotropy = std::nullopt,
       .borderColor = vk::BorderColor::eFloatOpaqueBlack,
       .unnormalizedCoordinates = false,
       .compareOp = std::nullopt,
       .mip = {.mode = vk::SamplerMipmapMode::eLinear, .lodBias = 0, .minLod = 0, .maxLod = 1}});

  infoWindow
      .createChild<Image>(
          "img1",
          [&] {
            return (ImTextureID) ImGui_ImplVulkan_AddTexture(
                **testTextureSampler, **testTextureView,
                static_cast<VkImageLayout>(testTexture->getImage().getLayout()));
          },
          ImVec2{300, 300}, IsButton::Yes,
          [] {
            return std::pair(ImVec2{0, 0}, ImVec2{1, 1});
          })
      .addClickListener([] { std::cout << "Image clicked" << std::endl; });

  imgui->setStateFromConfig();
}
void pf::TriangleRenderer::updateCommandBuffer() {
  for (auto i : std::views::iota(0ul, vkCommandBuffers.size())) {
    auto clearValues = std::vector<vk::ClearValue>(2);
    clearValues[0].setColor({std::array<float, 4>{0.f, 1.f, 0.f, 0.f}});
    clearValues[1].setDepthStencil({1.f, 0});
    auto recorder = vkCommandBuffers[i]->begin(vk::CommandBufferUsageFlagBits::eRenderPassContinue);
    recorder
        .beginRenderPass({.renderPass = *vkRenderPass,
                          .frameBuffer = *vkSwapChain->getFrameBuffers()[i],
                          .clearValues = clearValues,
                          .extent = vkSwapChain->getExtent()})
        .bindPipeline(vk::PipelineBindPoint::eGraphics, *vkGraphicsPipeline)
        .draw({3, 1, 0, 0});
    imgui->addToCommandBuffer(recorder);

    recorder.endRenderPass();
  }
}
void pf::TriangleRenderer::render() {
  auto sampler = FlameGraphSampler();

  auto mainBlockSampler = sampler.blockSampler("main");

  auto swapSample = mainBlockSampler.blockSampler("swap");
  vkSwapChain->swap();
  swapSample.end();

  auto imguiSample = mainBlockSampler.blockSampler("imgui");
  imgui->render();
  imguiSample.end();

  auto renderSample = mainBlockSampler.blockSampler("render");
  updateCommandBuffer();
  auto fencSemSample = renderSample.blockSampler("fences");
  auto &semaphore = vkSwapChain->getCurrentSemaphore();
  auto &fence = vkSwapChain->getCurrentFence();
  const auto commandBufferIndex = vkSwapChain->getCurrentImageIndex();
  const auto frameIndex = vkSwapChain->getCurrentFrameIndex();

  fence.reset();
  fencSemSample.end();

  auto submitSample = renderSample.blockSampler("submit");
  vkCommandBuffers[commandBufferIndex]->submit(
      {.waitSemaphores = {semaphore},
       .signalSemaphores = {*renderSemaphores[frameIndex]},
       .flags = vk::PipelineStageFlagBits::eColorAttachmentOutput,
       .fence = fence,
       .wait = true});
  submitSample.end();

  auto presentSample = renderSample.blockSampler("present");
  vkSwapChain->present(vulkan::PresentConfig{.waitSemaphores = {*renderSemaphores[frameIndex]},
                                             .presentQueue = vkLogicalDevice->getPresentQueue()});
  presentSample.end();
  vkSwapChain->frameDone();
  renderSample.end();
  fpsCounter.onFrame();
  mainBlockSampler.end();
  statsFlameGraph->setSamples(sampler.getSamples());
}
