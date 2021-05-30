//
// Created by petr on 5/21/21.
//

#include "SimpleSVORenderer_UI.h"
#include <logging/loggers.h>
#include <pf_imgui/elements/Bullet.h>
#include <pf_imgui/elements/Slider2D.h>
#include <pf_imgui/interface/decorators/WidthDecorator.h>
#include <pf_imgui/styles/dark.h>

namespace pf {
using namespace ui::ig;
SimpleSVORenderer_UI::SimpleSVORenderer_UI(std::unique_ptr<ui::ig::ImGuiGlfwVulkan> &&imguiInterface,
                                           const Camera &camera, TextureData iterTextureData)
    : imgui(std::move(imguiInterface)), windowMenuBar(imgui->getMenuBar()),
      fileSubMenu(windowMenuBar.addSubmenu("file_main_menu", "File")),
      openModelMenuItem(fileSubMenu.addButtonItem("open_model_menu", "Open model")),
      fileMenuSeparator1(fileSubMenu.addSeparator("fileMenuSeparator1")),
      closeMenuItem(fileSubMenu.addButtonItem("file_close_menu", "Close")),
      viewSubMenu(windowMenuBar.addSubmenu("view_main_menu", "View")),
      infoMenuItem(viewSubMenu.addCheckboxItem("view_info_menu", "Info", true)),
      renderSettingsMenuItem(viewSubMenu.addCheckboxItem("view_render_settings_menu", "Render settings", true)),
      debugMenuItem(viewSubMenu.addCheckboxItem("view_debug_menu", "Debug", true)),
      debugImagesMenuItem(viewSubMenu.addCheckboxItem("view_debug_images_menu", "Debug images", true)),
      shaderControlsMenuItem(viewSubMenu.addCheckboxItem("view_shader_controls_menu", "Shader controls", true)),
      modelsMenuItem(viewSubMenu.addCheckboxItem("view_models_menu", "Models", true)),
      separatorMenu1(viewSubMenu.addSeparator("separator_menu_1")),
      hideAllMenuItem(viewSubMenu.addButtonItem("hide_all_windows_menu", "Hide all")),
      showAllMenuItem(viewSubMenu.addButtonItem("show_all_windows_menu", "Show all")),
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
      fpsCurrentPlot(infoWindow.createChild<SimplePlot>("fps_plot", "Fps current", PlotType::Lines,
                                                        std::vector<float>{}, std::nullopt, 200, 0, FLT_MAX,
                                                        Size{Width::Auto(), 30})),
      fpsAveragePlot(infoWindow.createChild<SimplePlot>("fps_avg_plot", "Fps average", PlotType::Lines,
                                                        std::vector<float>{}, std::nullopt, 200, 0, FLT_MAX,
                                                        Size{Width::Auto(), 30})),
      fpsLabel(infoWindow.createChild<Text>("fps_label", "FPS")),
      resetFpsButton(infoWindow.createChild<Button>("fps_reset_button", "Reset FPS")),
      vsyncCheckbox(
          infoWindow.createChild<Checkbox>("vsync_chckbx", "Vsync", Checkbox::Type::Toggle, true, Persistent::Yes)),
      flameGraph(infoWindow.createChild<FlameGraph>("perf_flamegraph", "Render loop", Size::Auto())),
      cameraGroup(infoWindow.createChild<Group>("camera_group", "Camera", Persistent::Yes, AllowCollapse::Yes)),
      cameraPosText(cameraGroup.createChild<Text>("camera_pos_text", "")),
      cameraDirText(cameraGroup.createChild<Text>("camera_dir_text", "")),
      cameraMoveSpeedSlider(cameraGroup.createChild<Slider<float>>("cameraMoveSpeedSlider", "Movement speed", 0.1f,
                                                                   50.f, camera.getMovementSpeed(), Persistent::Yes)),
      cameraMouseSpeedSlider(cameraGroup.createChild<Slider<float>>("cameraMouseSpeedSlider", "Mouse speed", 0.1f, 50.f,
                                                                    camera.getMouseSpeed(), Persistent::Yes)),
      cameraFOVSlider(cameraGroup.createChild<Slider<int>>("cameraFOVSlider", "Field of view", 1, 179,
                                                           camera.getFieldOfView(), Persistent::Yes)),
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
      shaderDebugValueInput(shaderControlsWindow.createChild<SpinInput<int>>("shader_int1_drag", "Shader debug val 1",
                                                                             1, 8, 1, 1, 10, Persistent::Yes)),
      shaderDebugFloatValueSlider(shaderControlsWindow.createChild<DragInput<float>>(
          "shader_float1_spin", "Shader float debug val", 0.01, -100, 100, 1)),
      shaderDebugIterDivideDrag(shaderControlsWindow.createChild<DragInput<float>>(
          "shader_iter_divide_drag", "Iteration view divider", 1, 1, 1024, 64, Persistent::Yes)),
      shaderSeparator1(shaderControlsWindow.createChild<Separator>("separator_shader")),
      shaderDebugTranslateDrag(shaderControlsWindow.createChild<DragInput<glm::vec3>>(
          "shader_translate_drag", "Translation", 0.01, -10, 10, glm::vec3{0})),
      shaderDebugRotateDrag(shaderControlsWindow.createChild<DragInput<glm::vec3>>("shader_rotate_drag", "Rotation",
                                                                                   0.05, -180, 180, glm::vec3{0})),
      shaderDebugScaleDrag(shaderControlsWindow.createChild<DragInput<glm::vec3>>("shader_scale_drag", "Scale", 0.05,
                                                                                  0.01, 10, glm::vec3{1, 1, 1})),
      modelsWindow(imgui->createWindow("models_window", "Models")),
      modelListsLayout(modelsWindow.createChild<AbsoluteLayout>("models_layout", Size{Width::Auto(), Height(170)})),
      modelList(modelListsLayout.createChild<WidthDecorator<ListBox<ModelInfo>>>(
          "models_list", ImVec2{10, 10}, Width{200}, "Models", std::vector<ModelInfo>{}, 0, 5)),
      modelsFilterInput(modelListsLayout.createChild<WidthDecorator<InputText>>("models_filter", ImVec2{10, 110},
                                                                                Width{200}, "Filter")),
      reloadModelListButton(
          modelListsLayout.createChild<Button>("model_list_reload", ImVec2{10, 130}, "Reload models")),
      activateSelectedModelButton(modelListsLayout.createChild<Button>("activate_selected_model_button",
                                                                       ImVec2{265, 40}, "", ButtonType::ArrowRight)),
      activeModelList(modelListsLayout.createChild<WidthDecorator<ListBox<ModelInfo>>>(
          "active_models_list", ImVec2{290, 10}, Width{200}, "Active models", std::vector<ModelInfo>{}, 0, 5)),
      removeSelectedActiveModelButton(
          modelListsLayout.createChild<Button>("remove_selected_active_model_button", ImVec2{290, 110}, "Remove")),
      modelDetailTitle(modelsWindow.createChild<Text>("model_detail_title", "Detail")),
      modelDetailLayout(modelsWindow.createChild<BoxLayout>("model_detail_layout", LayoutDirection::TopToBottom,
                                                            Size{Width::Auto(), 185})),
      modelDetailPathText(modelDetailLayout.createChild<InputText>("model_detail_path", "Path", "{}")),
      modelDetailSVOHeightText(modelDetailLayout.createChild<InputText>("model_detail_svo_height", "SVO height", "")),
      modelDetailVoxelCountText(
          modelDetailLayout.createChild<InputText>("model_detail_voxel_count", "Voxel count", "")),
      modelDetailMinimisedVoxelCountText(
          modelDetailLayout.createChild<InputText>("model_detail_min_voxel_count", "Minimised voxel count", "")),
      modelDetailSeparator1(modelDetailLayout.createChild<Separator>("model_detail_separator_1")),
      modelDetailTranslateDrag(modelDetailLayout.createChild<DragInput<glm::vec3>>(
          "model_detail_translate_drag", "Translation", 0.01, -100, 100, glm::vec3{0})),
      modelDetailRotateDrag(modelDetailLayout.createChild<DragInput<glm::vec3>>("model_detail_rotate_drag", "Rotate",
                                                                                0.01, -180, 180, glm::vec3{0})),
      modelDetailScaleDrag(modelDetailLayout.createChild<DragInput<glm::vec3>>("model_detail_scale_drag", "Scale", 0.01,
                                                                               0.01, 10, glm::vec3{1, 1, 1}))

{
  setDarkStyle(*imgui);
  modelDetailLayout.setDrawBorder(true);
  modelDetailPathText.setReadOnly(true);
  modelDetailSVOHeightText.setReadOnly(true);
  modelDetailVoxelCountText.setReadOnly(true);
  modelDetailMinimisedVoxelCountText.setReadOnly(true);
  modelList.setDragTooltip("Model: {}");
  activeModelList.addValueListener([this](const auto &modelInfo) {
    modelDetailPathText.setText(modelInfo.path.string());
    modelDetailSVOHeightText.setText("{}", modelInfo.svoHeight);
    modelDetailVoxelCountText.setText("{}", modelInfo.voxelCount);
    modelDetailMinimisedVoxelCountText.setText("{}", modelInfo.minimizedVoxelCount);
    modelDetailTranslateDrag.setValue(modelInfo.translateVec);
    modelDetailRotateDrag.setValue(modelInfo.rotateVec);
    modelDetailScaleDrag.setValue(modelInfo.scaleVec);
  });
  modelDetailTranslateDrag.addValueListener([this](const auto &val) {
    if (auto selectedItem = activeModelList.getSelectedItem(); selectedItem.has_value()) {
      auto items = activeModelList.getItems();
      for (auto &item : items) {
        if (item == *selectedItem) { item.translateVec = val; }
      }
    }
  });
  modelDetailRotateDrag.addValueListener([this](const auto &val) {
    if (auto selectedItem = activeModelList.getSelectedItem(); selectedItem.has_value()) {
      auto items = activeModelList.getItems();
      for (auto &item : items) {
        if (item == *selectedItem) { item.rotateVec = val; }
      }
    }
  });
  modelDetailScaleDrag.addValueListener([this](const auto &val) {
    if (auto selectedItem = activeModelList.getSelectedItem(); selectedItem.has_value()) {
      auto items = activeModelList.getItems();
      for (auto &item : items) {
        if (item == *selectedItem) { item.scaleVec = val; }
      }
    }
  });

  iterationImage.setTooltip("Visualisation of ray iterations");

  infoMenuItem.addValueListener(
      [this](auto value) { infoWindow.setVisibility(value ? Visibility::Visible : Visibility::Invisible); });
  renderSettingsMenuItem.addValueListener(
      [this](auto value) { renderSettingsWindow.setVisibility(value ? Visibility::Visible : Visibility::Invisible); });
  debugMenuItem.addValueListener(
      [this](auto value) { debugWindow.setVisibility(value ? Visibility::Visible : Visibility::Invisible); });
  debugImagesMenuItem.addValueListener(
      [this](auto value) { debugImagesWindow.setVisibility(value ? Visibility::Visible : Visibility::Invisible); });
  shaderControlsMenuItem.addValueListener(
      [this](auto value) { shaderControlsWindow.setVisibility(value ? Visibility::Visible : Visibility::Invisible); });
  modelsMenuItem.addValueListener(
      [this](auto value) { modelsWindow.setVisibility(value ? Visibility::Visible : Visibility::Invisible); });
  hideAllMenuItem.addClickListener([this] { setWindowsVisible(false); });
  showAllMenuItem.addClickListener([this] { setWindowsVisible(true); });

  infoMenuItem.setCloseOnInteract(false);
  renderSettingsMenuItem.setCloseOnInteract(false);
  debugMenuItem.setCloseOnInteract(false);
  debugImagesMenuItem.setCloseOnInteract(false);
  shaderControlsMenuItem.setCloseOnInteract(false);
  modelsMenuItem.setCloseOnInteract(false);

  renderSettingsWindow.setCollapsible(true);
  renderSettingsWindow.setCloseable(true);
  renderSettingsWindow.addCloseListener([this] { renderSettingsMenuItem.setValue(false); });
  infoWindow.setCollapsible(true);
  infoWindow.setCloseable(true);
  infoWindow.addCloseListener([this] { infoMenuItem.setValue(false); });
  debugImagesWindow.setCollapsible(true);
  debugImagesWindow.setCloseable(true);
  debugImagesWindow.addCloseListener([this] { debugImagesMenuItem.setValue(false); });
  shaderControlsWindow.setCollapsible(true);
  shaderControlsWindow.setCloseable(true);
  shaderControlsWindow.addCloseListener([this] { shaderControlsMenuItem.setValue(false); });
  debugWindow.setCollapsible(true);
  debugWindow.setCloseable(true);
  debugWindow.addCloseListener([this] { debugMenuItem.setValue(false); });
  modelsWindow.setCollapsible(true);
  modelsWindow.setCloseable(true);
  modelsWindow.addCloseListener([this] { modelsMenuItem.setValue(false); });
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

  //openModelButton.setTooltip("Select model file via file explorer");

  modelList.setTooltip("Models from model folder");

  modelsFilterInput.setTooltip("Filter list of models");
  reloadModelListButton.setTooltip("Reload models from model folder (config file)");
  activateSelectedModelButton.setTooltip("Add selected model to scene");
  activeModelList.setTooltip("Models in scene");
  removeSelectedActiveModelButton.setTooltip("Remove selected model from scene");

  logTab.setTooltip("Program logs");
  chaiscriptTab.setTooltip("Chaiscript interface");
  resetFpsButton.setTooltip("Reset FPS counters");
  vsyncCheckbox.setTooltip("Enable/disable vsync, CURRENTLY NOT WORKING");

  cameraMoveSpeedSlider.setTooltip("Camera movement speed with WASD");
  cameraMouseSpeedSlider.setTooltip("Camera pan speed with mouse");
  cameraFOVSlider.setTooltip("Camera field of view");

  modelList.setDragAllowed(true);
  activeModelList.setDropAllowed(true);
  modelListsLayout.setDrawBorder(true);
}

void SimpleSVORenderer_UI::setWindowsVisible(bool visible) {
  infoWindow.setVisibility(visible ? Visibility::Visible : Visibility::Invisible);
  renderSettingsWindow.setVisibility(visible ? Visibility::Visible : Visibility::Invisible);
  debugWindow.setVisibility(visible ? Visibility::Visible : Visibility::Invisible);
  debugImagesWindow.setVisibility(visible ? Visibility::Visible : Visibility::Invisible);
  shaderControlsWindow.setVisibility(visible ? Visibility::Visible : Visibility::Invisible);
  modelsWindow.setVisibility(visible ? Visibility::Visible : Visibility::Invisible);
  infoMenuItem.setValue(visible);
  renderSettingsMenuItem.setValue(visible);
  debugMenuItem.setValue(visible);
  debugImagesMenuItem.setValue(visible);
  shaderControlsMenuItem.setValue(visible);
  modelsMenuItem.setValue(visible);
}

std::ostream &operator<<(std::ostream &os, const ModelInfo &info) {
  os << info.path.filename().string();
  return os;
}
bool ModelInfo::operator==(const ModelInfo &rhs) const { return id == rhs.id; }
bool ModelInfo::operator!=(const ModelInfo &rhs) const { return !(rhs == *this); }
}// namespace pf
