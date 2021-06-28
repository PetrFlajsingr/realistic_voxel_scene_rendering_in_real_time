//
// Created by petr on 5/21/21.
//

#include "SimpleSVORenderer_UI.h"
#include <logging/loggers.h>
#include <pf_imgui/elements/Bullet.h>
#include <pf_imgui/elements/Slider2D.h>
#include <pf_imgui/interface/decorators/WidthDecorator.h>
#include <pf_imgui/styles/dark.h>
#include <pf_imgui/backends/impl/imgui_impl_vulkan.h>
#include <string>
#include <utility>

namespace pf {
using namespace ui::ig;
ModelFileInfo::ModelFileInfo(std::filesystem::path path) : path(std::move(path)) {}
bool ModelFileInfo::operator==(const ModelFileInfo &rhs) const { return id == rhs.id; }
bool ModelFileInfo::operator!=(const ModelFileInfo &rhs) const { return !(rhs == *this); }

std::ostream &operator<<(std::ostream &os, const ModelFileInfo &info) {
  os << info.path.filename().string();
  if (info.modelData != nullptr) { os << std::to_string(info.id); }
  return os;
}

SimpleSVORenderer_UI::SimpleSVORenderer_UI(std::unique_ptr<ui::ig::ImGuiGlfwVulkanInterface> &&imguiInterface,
                                           std::shared_ptr<ui::Window> uiWindow, const Camera &camera,
                                           TextureData iterTextureData, TextureData probesColorTextureData)
    : imgui(std::move(imguiInterface)), window(std::move(uiWindow)), windowMenuBar(imgui->getMenuBar()),
      fileSubMenu(windowMenuBar.addSubmenu("file_main_menu", "File")),
      openModelMenuItem(fileSubMenu.addButtonItem("open_model_menu", ICON_FA_FILE_ALT "  Open model")),
      loadSceneMenuItem(fileSubMenu.addButtonItem("load_scene_menu", ICON_FA_FILE_ALT "  Load scene")),
      saveSceneMenuItem(fileSubMenu.addButtonItem("save_scene_menu", ICON_FA_SAVE "  Save scene")),
      fileMenuSeparator1(fileSubMenu.addSeparator("fileMenuSeparator1")),
      closeMenuItem(fileSubMenu.addButtonItem("file_close_menu", ICON_FA_WINDOW_CLOSE "  Close")),
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
      toolsSubMenu(windowMenuBar.addSubmenu("tools_main_menu", "Tools")),
      svoConverterMenuItem(toolsSubMenu.addButtonItem("svo_converter_menu", "SVO converter")),
      teardownMapMenuItem(toolsSubMenu.addButtonItem("teardown_converter_menu", "Teardown map loading")),
      renderSettingsWindow(imgui->createWindow("render_sett_window", "Render settings")),
      viewTypeComboBox(renderSettingsWindow.createChild<Combobox<ViewType>>(
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
      cameraToOriginButton(cameraGroup.createChild<Button>("camera_to_origin_button", "Move to origin")),
      cameraMoveSpeedSlider(cameraGroup.createChild<Slider<float>>("cameraMoveSpeedSlider", "Movement speed", 0.1f,
                                                                   50.f, camera.getMovementSpeed(), Persistent::Yes)),
      cameraMouseSpeedSlider(cameraGroup.createChild<Slider<float>>("cameraMouseSpeedSlider", "Mouse speed", 0.1f, 50.f,
                                                                    camera.getMouseSpeed(), Persistent::Yes)),
      cameraFOVSlider(cameraGroup.createChild<Slider<int>>("cameraFOVSlider", "Field of view", 1, 179,
                                                           camera.getFieldOfView(), Persistent::Yes)),
      sceneGroup(infoWindow.createChild<Group>("scene_info_group", "Scene", Persistent::Yes, AllowCollapse::Yes)),
      sceneModelCountText(sceneGroup.createChild<Bullet<Text>>("scene_model_count_text", "")),
      sceneVoxelCountText(sceneGroup.createChild<Bullet<Text>>("voxel_count_text", "")),
      sceneBVHNodeCountText(sceneGroup.createChild<Bullet<Text>>("scene_bvh_node_count_text", "")),
      sceneBVHDepthText(sceneGroup.createChild<Bullet<Text>>("scene_bvh_depth_text", "")),
      debugImagesWindow(imgui->createWindow("debug_images_window", "Debug images")),
      imageStretchLayout(
          debugImagesWindow.createChild<StretchLayout>("iter_image_stretch_layout", Size::Auto(), Stretch::All)),
      iterationImage(imageStretchLayout.createChild<pf::ui::ig::Image>(
          "iter_image",
          (ImTextureID) ImGui_ImplVulkan_AddTexture(*iterTextureData.vkImageSampler, *iterTextureData.vkImageView,
                                                    static_cast<VkImageLayout>(iterTextureData.vkImage.getLayout())),
          Size::Auto())),
      shaderControlsWindow(imgui->createWindow("shader_controls_window", "Shader controls")),
      debugPrintEnableCheckbox(shaderControlsWindow.createChild<Checkbox>("debug_print_enabled", "Enable debug print")),
      bvhVisualizeCheckbox(
          shaderControlsWindow.createChild<Checkbox>("bvh_visualisation_enabled", "Enable BVH visualisation")),
      visualizeProbesCheckbox(
          shaderControlsWindow.createChild<Checkbox>("probe_visualisation_enabled", "Enable LF probe visualisation")),
      shaderDebugValueInput(shaderControlsWindow.createChild<SpinInput<int>>("shader_int1_drag", "Shader debug val 1",
                                                                             1, 8, 1, 1, 10, Persistent::Yes)),
      shaderDebugFloatValueSlider(shaderControlsWindow.createChild<DragInput<float>>(
          "shader_float1_spin", "Shader float debug val", 0.01, -100, 100, 1)),
      shaderDebugIterDivideDrag(shaderControlsWindow.createChild<DragInput<float>>(
          "shader_iter_divide_drag", "Iteration view divider", 1, 1, 1024, 64, Persistent::Yes)),
      modelsWindow(imgui->createWindow("models_window", "Models")),
      modelLoadingSettingsTitle(modelsWindow.createChild<Text>("loading_settings_title", "Loading settings:")),
      modelLoadingSettings(
          modelsWindow.createChild<BoxLayout>("loading_settings_layout", LayoutDirection::LeftToRight, Size{280, 20})),
      modelLoadingSeparateModelsCheckbox(modelLoadingSettings.createChild<Checkbox>(
          "loading_separate_models_checkbox", "Separate", false, Persistent::Yes)),
      modelListsLayout(modelsWindow.createChild<AbsoluteLayout>("models_layout", Size{Width::Auto(), Height(170)})),
      modelList(modelListsLayout.createChild<Listbox<ModelFileInfo>>("models_list", ImVec2{10, 10}, "Models",
                                                                     Size{200, 100}, std::nullopt, Persistent::Yes)),
      modelsFilterInput(modelListsLayout.createChild<WidthDecorator<InputText>>("models_filter", ImVec2{10, 115},
                                                                                Width{200}, "Filter")),
      reloadModelListButton(
          modelListsLayout.createChild<Button>("model_list_reload", ImVec2{10, 140}, "Reload models")),
      activateSelectedModelButton(modelListsLayout.createChild<Button>("activate_selected_model_button",
                                                                       ImVec2{265, 40}, "", ButtonType::ArrowRight)),
      activeModelList(modelListsLayout.createChild<Listbox<ModelFileInfo>>(
          "active_models_list", ImVec2{290, 10}, "Active models", Size{200, 100}, std::nullopt)),
      activeModelsLayout(modelListsLayout.createChild<BoxLayout>("active_model_buttons_layout", ImVec2{285, 115},
                                                                 LayoutDirection::LeftToRight, Size{280, 50})),
      clearActiveModelsButton(activeModelsLayout.createChild<Button>("clear_active_model_button", "Clear")),
      activeModelsFilterInput(activeModelsLayout.createChild<InputText>("active_models_filter", "Filter", "")),
      modelDetailTitle(modelsWindow.createChild<Text>("model_detail_title", "Detail")),
      modelDetailLayout(modelsWindow.createChild<BoxLayout>("model_detail_layout", LayoutDirection::TopToBottom,
                                                            Size{Width::Auto(), 185})),
      modelDetailIIDText(modelDetailLayout.createChild<InputText>("model_detail_id", "Model ID", "{}")),
      modelDetailPathText(modelDetailLayout.createChild<InputText>("model_detail_path", "Path", "{}")),
      modelDetailSVOHeightText(modelDetailLayout.createChild<InputText>("model_detail_svo_height", "SVO height", "")),
      modelDetailVoxelCountText(
          modelDetailLayout.createChild<InputText>("model_detail_voxel_count", "Voxel count", "")),
      modelDetailMinimisedVoxelCountText(
          modelDetailLayout.createChild<InputText>("model_detail_min_voxel_count", "Minimised voxel count", "")),
      modelDetailSeparator1(modelDetailLayout.createChild<Separator>("model_detail_separator_1")),
      modelDetailBufferInfo(modelDetailLayout.createChild<Text>("model_detail_buffer_info", "Buffer info:")),
      modelDetailBufferOffset(
          modelDetailLayout.createChild<Bullet<Text>>("model_detail_buffer_offset", MODEL_BUFFER_OFFSET_INFO)),
      modelDetailBufferSize(
          modelDetailLayout.createChild<Bullet<Text>>("model_detail_buffer_size", MODEL_BUFFER_SIZE_INFO)),
      modelDetailTranslateDrag(modelDetailLayout.createChild<DragInput<glm::vec3>>(
          "model_detail_translate_drag", "Translation", 0.01, -100, 100, glm::vec3{0})),
      modelDetailRotateDrag(modelDetailLayout.createChild<DragInput<glm::vec3>>("model_detail_rotate_drag", "Rotate",
                                                                                0.01, -180, 180, glm::vec3{0})),
      modelDetailScaleDrag(modelDetailLayout.createChild<DragInput<glm::vec3>>("model_detail_scale_drag", "Scale", 0.01,
                                                                               0.01, 10, glm::vec3{1, 1, 1})),
      probesDebugWindow(imgui->createWindow("probes_debug_window", "Probes debug")),
      renderProbesButton(probesDebugWindow.createChild<Button>("render_probes_button", "Render probes in next pass")),
      selectedProbeSpinner(probesDebugWindow.createChild<SpinInput<int>>("selected_prob_spinner", "Selected probe",
                                                                              0, 64, 0, 1, 10)),
      probesDebugIntSpinner(probesDebugWindow.createChild<SpinInput<int>>("probes_debug_int_spinner", "Debug int",
                                                                              0, std::numeric_limits<int>::max(), 0, 1, 10)),
      probesTabBar(probesDebugWindow.createChild<TabBar>("probe_debug_tabbar")),
      probesTexturesTab(probesTabBar.addTab("probes_debug_textures_tab", "Textures")),
      probeTextureCombobox(probesTexturesTab.createChild<Combobox<ProbeVisualisation>>(
          "probe_texture_type_combobox", "Data type", "Select data type", magic_enum::enum_values<ProbeVisualisation>(),
          ComboBoxCount::Items8, Persistent::Yes)),
      probeTextureLayout(
          probesTexturesTab.createChild<StretchLayout>("probe_image_stretch_layout", Size::Auto(), Stretch::All)),
      probesColorImage(probeTextureLayout.createChild<Image>(
          "probes_color_texture",
          (ImTextureID) ImGui_ImplVulkan_AddTexture(
              *probesColorTextureData.vkImageSampler, *probesColorTextureData.vkImageView,
              static_cast<VkImageLayout>(probesColorTextureData.vkImage.getLayout())),
          Size{400, 400}))

{
  setDarkStyle(*imgui);
  modelDetailLayout.setDrawBorder(true);
  modelDetailLayout.setScrollable(true);
  modelDetailIIDText.setReadOnly(true);
  modelDetailPathText.setReadOnly(true);
  modelDetailSVOHeightText.setReadOnly(true);
  modelDetailVoxelCountText.setReadOnly(true);
  modelDetailMinimisedVoxelCountText.setReadOnly(true);
  modelList.setDragTooltip("Model: {}");

  probesDebugWindow.setCollapsible(true);

  renderProbesButton.setTooltip("Render G buffer for each probe in the next pass");
  modelLoadingSeparateModelsCheckbox.setTooltip("Load models in model file as separate SVOs");

  activeModelList.addValueListener([this](const auto &modelInfo) {
    modelDetailIIDText.setText("{}", modelInfo.modelData->getModelIndex().value());
    modelDetailPathText.setText(modelInfo.modelData->path.string());
    modelDetailSVOHeightText.setText("{}", modelInfo.modelData->svoHeight);
    modelDetailVoxelCountText.setText("{}", modelInfo.modelData->voxelCount);
    modelDetailMinimisedVoxelCountText.setText("{}", modelInfo.modelData->minimizedVoxelCount);
    modelDetailTranslateDrag.setValue(modelInfo.modelData->translateVec);
    modelDetailRotateDrag.setValue(modelInfo.modelData->rotateVec);
    modelDetailScaleDrag.setValue(modelInfo.modelData->scaleVec);
    modelDetailBufferOffset.setText(MODEL_BUFFER_OFFSET_INFO, modelInfo.modelData->svoMemoryBlock->getOffset());
    modelDetailBufferSize.setText(MODEL_BUFFER_SIZE_INFO, modelInfo.modelData->svoMemoryBlock->getSize());
  });

  clearActiveModelsButton.setTooltip("Clear all models from the scene");

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
  activeModelList.setTooltip("Models in scene - right click for more options");
  activeModelsFilterInput.setTooltip("Filter list of active models");

  modelsFilterInput.addValueListener([this](auto filterVal) {
    modelList.setFilter([filterVal](const auto &item) { return toString(item).find(filterVal) != std::string::npos; });
  });

  activeModelsFilterInput.addValueListener([this](auto filterVal) {
    activeModelList.setFilter(
        [filterVal](const auto &item) { return toString(item).find(filterVal) != std::string::npos; });
  });

  logTab.setTooltip("Program logs");
  chaiscriptTab.setTooltip("Chaiscript interface");
  resetFpsButton.setTooltip("Reset FPS counters");
  vsyncCheckbox.setTooltip("Enable/disable vsync, CURRENTLY NOT WORKING");

  cameraToOriginButton.setTooltip("Move camera to [0, 0, 0]");
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

std::tuple<ui::ig::ModalDialog &, ui::ig::ProgressBar<float> &, ui::ig::Text &>
SimpleSVORenderer_UI::createLoadingDialog() {
  auto &loadingDialog = imgui->createDialog(uniqueId(), "Loading model...");
  loadingDialog.setSize(Size{200, 100});
  auto &loadingProgressBar = loadingDialog.createChild<ProgressBar<float>>(uniqueId(), 1, 0, 100, 0);
  auto &msgText = loadingDialog.createChild<Text>(uniqueId(), "");
  return std::tuple<ui::ig::ModalDialog &, ui::ig::ProgressBar<float> &, ui::ig::Text &>{loadingDialog,
                                                                                         loadingProgressBar, msgText};
}
}// namespace pf
