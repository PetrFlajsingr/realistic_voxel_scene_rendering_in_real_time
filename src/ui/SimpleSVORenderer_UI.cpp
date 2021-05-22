//
// Created by petr on 5/21/21.
//

#include "SimpleSVORenderer_UI.h"
#include <pf_imgui/elements/Bullet.h>
#include <pf_imgui/elements/Slider2D.h>
#include <pf_imgui/styles/dark.h>

#include <logging/loggers.h>

namespace pf {
using namespace ui::ig;
SimpleSVORenderer_UI::SimpleSVORenderer_UI(std::unique_ptr<ui::ig::ImGuiGlfwVulkan> &&imguiInterface,
                                           const Camera &camera, TextureData iterTextureData)
    : imgui(std::move(imguiInterface)),
      renderSettingsWindow(imgui->createWindow("render_sett_window", "Render settings")),
      viewTypeComboBox(renderSettingsWindow.createChild<ComboBox<ViewType>>(
          "view_choice", "View type", "Select view type", magic_enum::enum_values<ViewType>(), ComboBoxCount::Items8,
          Persistent::Yes)),
      lightingText(renderSettingsWindow.createChild<Text>("lighting_header", "Lighting:")),
      lightingLayout(renderSettingsWindow.createChild<BoxLayout>("lighting", LayoutDirection::TopToBottom,
                                                                 Size{Width::Auto(), 320}, ShowBorder::Yes)),
      lightPosSlider(lightingLayout.createChild<Slider3D<float>>(
          "slider_lightpos", "Light position", glm::vec2{-100, 100}, glm::vec2{-100, 100}, glm::vec2{-100, 100},
          glm::vec3{}, Size{100, 100}, Persistent::Yes)),
      shadowsCheckbox(lightingLayout.createChild<Checkbox>("check_shadows", "Shadows", Checkbox::Type::Toggle, false,
                                                           Persistent::Yes)),
      phongParamText(lightingLayout.createChild<Text>("phong_params_header", "Phong parameters:")),
      phongParamLayout(lightingLayout.createChild<BoxLayout>("phong_params_layout", LayoutDirection::TopToBottom,
                                                             Size{Width::Auto(), 90}, ShowBorder::Yes)),
      ambientColPicker(phongParamLayout.createChild<ColorEdit<glm::vec3>>("picker_light_ambient", "Ambient",
                                                                          glm::vec3{0.1f}, Persistent::Yes)),
      diffuseColPicker(phongParamLayout.createChild<ColorEdit<glm::vec3>>("picker_light_diffuse", "Diffuse",
                                                                          glm::vec3{0.6f}, Persistent::Yes)),
      specularColPicker(phongParamLayout.createChild<ColorEdit<glm::vec3>>("picker_light_specular", "Specular",
                                                                           glm::vec3{0.9f}, Persistent::Yes)),
      modelsText(renderSettingsWindow.createChild<Text>("models_header", "Models:")),
      modelsLayout(renderSettingsWindow.createChild<BoxLayout>("models_layout", LayoutDirection::TopToBottom,
                                                               Size{Width::Auto(), 180}, ShowBorder::Yes)),
      openModelButton(modelsLayout.createChild<Button>("open_model_btn", "Open model")),
      modelList(modelsLayout.createChild<ListBox<ModelInfo>>("models_list", "Models", std::vector<ModelInfo>{}, 0, 5,
                                                             Persistent::Yes)),
      modelsFilterInput(modelsLayout.createChild<InputText>("models_filter", "Filter")),
      modelReloadButtonsLayout(
          modelsLayout.createChild<BoxLayout>("models_buttons", LayoutDirection::LeftToRight, Size{Width::Auto(), 20})),
      reloadModelListButton(modelReloadButtonsLayout.createChild<Button>("model_list_reload", "Reload models")),
      reloadSelectedModelButton(modelReloadButtonsLayout.createChild<Button>("model_reload", "Reload selected")),
      debugWindow(imgui->createWindow("debug_window", "Debug")),
      debugTabBar(debugWindow.createChild<TabBar>("debug_tabbar")), logTab(debugTabBar.addTab("log_tab", "Log")),
      logMemo(logTab.createChild<Memo>("log_output", "Log:", 100, true, true, 100)),
      logErrMemo(logTab.createChild<Memo>("log_err_output", "Log: err", 100, true, true, 100)),
      chaiscriptTab(debugTabBar.addTab("chai_tab", "ChaiScript")),
      chaiInputLayout(chaiscriptTab.createChild<BoxLayout>("chai_input_panel", LayoutDirection::LeftToRight,
                                                           Size{Width::Auto(), 120})),
      chaiInputLabel(chaiInputLayout.createChild<Text>("chain_input_label", "Input:")),
      chaiInputText(chaiInputLayout.createChild<InputText>("chai_input", "", "", TextInputType::MultiLine)),
      chaiOutputMemo(chaiscriptTab.createChild<Memo>("chai_output", "Output:", 100, true, true, 100)),
      chaiConfirmButton(chaiInputLayout.createChild<Button>("chain_input_confirm", "Confirm")),
      infoWindow(imgui->createWindow("info_window", "Info")),
      fpsPlot(infoWindow.createChild<SimplePlot>("fps_plot", "Fps", PlotType::Lines, std::vector<float>{}, std::nullopt,
                                                 200, 0, 60, Size{Width::Auto(), 50})),
      fpsLabel(infoWindow.createChild<Text>("fps_label", "FPS")),
      resetFpsButton(infoWindow.createChild<Button>("fps_reset_button", "Reset FPS")),
      vsyncCheckbox(
          infoWindow.createChild<Checkbox>("vsync_chckbx", "Vsync", Checkbox::Type::Toggle, true, Persistent::Yes)),
      cameraGroup(infoWindow.createChild<Group>("camera_group", "Camera", Persistent::Yes, AllowCollapse::Yes)),
      cameraPosText(cameraGroup.createChild<Text>("camera_pos_text", "")),
      cameraDirText(cameraGroup.createChild<Text>("camera_dir_text", "")),
      cameraMoveSpeedSlider(cameraGroup.createChild<Slider<float>>("cameraMoveSpeedSlider", "Movement speed", 0.1f,
                                                                   50.f, camera.getMovementSpeed(), Persistent::Yes)),
      cameraMouseSpeedSlider(cameraGroup.createChild<Slider<float>>("cameraMouseSpeedSlider", "Mouse speed", 0.1f, 50.f,
                                                                    camera.getMouseSpeed(), Persistent::Yes)),
      cameraFOVSlider(cameraGroup.createChild<Slider<float>>("cameraFOVSlider", "Field of view", 1.f, 90.f,
                                                             camera.getFieldOfView(), Persistent::Yes)),
      sceneInfoGroup(infoWindow.createChild<Group>("scene_group", "Scene", Persistent::Yes, AllowCollapse::Yes)),
      modelNameText(sceneInfoGroup.createChild<Text>("model_name_text", "")),
      modelNameSeparator(sceneInfoGroup.createChild<Separator>(uniqueId())),
      svoHeightText(sceneInfoGroup.createChild<Bullet<Text>>("svo_height_text", SVO_HEIGHT_TEXT)),
      voxelCountText(sceneInfoGroup.createChild<Bullet<Text>>("voxel_count_text", VOXEL_COUNT_TEXT)),
      voxelCountMinimizedText(
          sceneInfoGroup.createChild<Bullet<Text>>("voxel_count_mini_text", VOXEL_COUNT_MINIMIZED_TEXT)),
      debugImagesWindow(imgui->createWindow("debug_images_window", "Debug images")),
      imageStretchLayout(
          debugImagesWindow.createChild<StretchLayout>("iter_image_stretch_layout", Size::Auto(), Stretch::All)),
      iterationImage(imageStretchLayout.createChild<pf::ui::ig::Image>(
          "iter_image",
          (ImTextureID) ImGui_ImplVulkan_AddTexture(
              *iterTextureData.vkIterImageSampler, *iterTextureData.vkIterImageView,
              static_cast<VkImageLayout>(iterTextureData.vkIterImage.getLayout())),
          Size::Auto())),
      shaderControlsWindow(imgui->createWindow("shader_controls_window", "Shader controls")),
      shaderDebugValueInput(shaderControlsWindow.createChild<DragInput<int>>("shader_int1_drag", "Shader debug val 1",
                                                                             1, 1, 10, 1, Persistent::Yes)) {
  setDarkStyle(*imgui);

  renderSettingsWindow.setCollapsible(true);
  infoWindow.setCollapsible(true);
  debugImagesWindow.setCollapsible(true);
  shaderControlsWindow.setCollapsible(true);
  debugWindow.setCollapsible(true);
  viewTypeComboBox.setTooltip("Render type for debug");
  lightPosSlider.setTooltip("Position of light point in the scene");
  shadowsCheckbox.setTooltip("Enable/disable shadows");
  phongParamLayout.setCollapsible(true);
  phongParamLayout.addCollapseListener([this](auto collapsed) {
    auto parentSize = lightingLayout.getSize();
    float change = phongParamLayout.getSize().height - 10;
    if (collapsed) { change *= -1; }
    parentSize.height.value += change;
    lightingLayout.setSize(parentSize);
  });

  ambientColPicker.setDragAllowed(true);
  ambientColPicker.setDropAllowed(true);
  ambientColPicker.setTooltip("Color of ambient light");

  diffuseColPicker.setDragAllowed(true);
  diffuseColPicker.setDropAllowed(true);
  diffuseColPicker.setTooltip("Color of diffuse light");

  specularColPicker.setDragAllowed(true);
  specularColPicker.setDropAllowed(true);
  specularColPicker.setTooltip("Color of specular light");

  openModelButton.setTooltip("Select model file via file explorer");

  modelList.setTooltip("Models from model folder");

  modelsFilterInput.setTooltip("Filter list of models");
  reloadModelListButton.setTooltip("Reload models from model folder (config file)");
  reloadSelectedModelButton.setTooltip("Reload currently selected model");

  logTab.setTooltip("Program logs");
  chaiscriptTab.setTooltip("Chaiscript interface");
  resetFpsButton.setTooltip("Reset FPS counters");
  vsyncCheckbox.setTooltip("Enable/disable vsync, CURRENTLY NOT WORKING");

}
void SimpleSVORenderer_UI::updateSceneInfo(const std::string &modelName, uint32_t svoHeight, uint32_t voxelCount,
                                           uint32_t miniVoxelCount) {
  modelNameText.setText(modelName);
  svoHeightText.setText(SVO_HEIGHT_TEXT, svoHeight);
  voxelCountText.setText(VOXEL_COUNT_TEXT, voxelCount);
  voxelCountMinimizedText.setText(VOXEL_COUNT_MINIMIZED_TEXT, miniVoxelCount);
}

std::ostream &operator<<(std::ostream &os, const ModelInfo &info) {
  os << info.path.filename().string();
  return os;
}
bool ModelInfo::operator==(const ModelInfo &rhs) const { return path == rhs.path; }
bool ModelInfo::operator!=(const ModelInfo &rhs) const { return !(rhs == *this); }
}// namespace pf