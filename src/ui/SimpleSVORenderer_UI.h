//
// Created by petr on 5/21/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UI_SIMPLESVORENDERER_UI_H
#define REALISTIC_VOXEL_RENDERING_SRC_UI_SIMPLESVORENDERER_UI_H

#include <glm/glm.hpp>
#include <ostream>
#include <pf_common/enums.h>
#include <pf_common/math/BoundingBox.h>
#include <pf_common/parallel/ThreadPool.h>
#include <pf_imgui/elements/Checkbox.h>
#include <pf_imgui/elements/ColorChooser.h>
#include <pf_imgui/elements/Combobox.h>
#include <pf_imgui/elements/DragInput.h>
#include <pf_imgui/elements/FlameGraph.h>
#include <pf_imgui/elements/Group.h>
#include <pf_imgui/elements/Image.h>
#include <pf_imgui/elements/InputText.h>
#include <pf_imgui/elements/Listbox.h>
#include <pf_imgui/elements/Memo.h>
#include <pf_imgui/elements/ProgressBar.h>
#include <pf_imgui/elements/Separator.h>
#include <pf_imgui/elements/Slider.h>
#include <pf_imgui/elements/Slider3D.h>
#include <pf_imgui/elements/SpinInput.h>
#include <pf_imgui/elements/TabBar.h>
#include <pf_imgui/elements/Text.h>
#include <pf_imgui/elements/plots/Plot.h>
#include <pf_imgui/elements/plots/SimplePlot.h>
#include <pf_imgui/elements/plots/types/Line.h>
#include <pf_imgui/layouts/AbsoluteLayout.h>
#include <pf_imgui/layouts/StretchLayout.h>
#include <range/v3/view/cache1.hpp>
#include <range/v3/view/join.hpp>
#include <tuple>
#include <ui/ImGuiGlfwVulkan.h>
#include <utils/Camera.h>
#include <voxel/GPUModelInfo.h>

namespace pf {

// TODO: move to separate header
enum class ViewType : int { Color = 0, Normals, Iterations, Distance, ChildIndex, TreeLevel };

// TODO: figure out why PF_ENUM_OUT doesn't work
inline std::ostream &operator<<(std::ostream &o, ViewType viewType) {
  o << magic_enum::enum_name(viewType);
  return o;
}

// TODO: terminal interface in pf_imgui

struct TextureData {
  vulkan::Image &vkIterImage;
  vulkan::ImageView &vkIterImageView;
  vulkan::TextureSampler &vkIterImageSampler;
};

struct ModelFileInfo {
  // TODO: group id and group controls
  ModelFileInfo() = default;
  ModelFileInfo(std::filesystem::path path);
  static inline auto IdGenerator = iota<std::size_t>();
  std::experimental::observer_ptr<vox::GPUModelInfo> modelData = nullptr;
  std::size_t id = getNext(IdGenerator);
  std::filesystem::path path{};
  bool operator==(const ModelFileInfo &rhs) const;
  bool operator!=(const ModelFileInfo &rhs) const;
  friend std::ostream &operator<<(std::ostream &os, const ModelFileInfo &info);
};

class SimpleSVORenderer_UI {
 public:
  explicit SimpleSVORenderer_UI(std::unique_ptr<ui::ig::ImGuiGlfwVulkan> &&imguiInterface,
                                std::shared_ptr<ui::Window> uiWindow, const Camera &camera,
                                TextureData iterTextureData);

  std::unique_ptr<ui::ig::ImGuiGlfwVulkan> imgui;
  std::shared_ptr<ui::Window> window;

  // clang-format off
  ui::ig::AppMenuBar &windowMenuBar;
    ui::ig::SubMenu &fileSubMenu;
      ui::ig::MenuButtonItem &openModelMenuItem;
      ui::ig::MenuButtonItem &loadSceneMenuItem;
      ui::ig::MenuButtonItem &saveSceneMenuItem;
      ui::ig::MenuSeparatorItem &fileMenuSeparator1;
      ui::ig::MenuButtonItem &closeMenuItem;
    ui::ig::SubMenu &viewSubMenu;
      ui::ig::MenuCheckboxItem &infoMenuItem;
      ui::ig::MenuCheckboxItem &renderSettingsMenuItem;
      ui::ig::MenuCheckboxItem &debugMenuItem;
      ui::ig::MenuCheckboxItem &debugImagesMenuItem;
      ui::ig::MenuCheckboxItem &shaderControlsMenuItem;
      ui::ig::MenuCheckboxItem &modelsMenuItem;
      ui::ig::MenuSeparatorItem &separatorMenu1;
      ui::ig::MenuButtonItem &hideAllMenuItem;
      ui::ig::MenuButtonItem &showAllMenuItem;
    ui::ig::SubMenu &toolsSubMenu;
      ui::ig::MenuButtonItem &svoConverterMenuItem;
  ui::ig::Window &renderSettingsWindow;
    ui::ig::Combobox<ViewType> &viewTypeComboBox;
    ui::ig::Text &lightingText;
    ui::ig::BoxLayout &lightingLayout;
      ui::ig::Slider3D<float> &lightPosSlider;
      ui::ig::Checkbox &shadowsCheckbox;
      ui::ig::Text &phongParamText;
      ui::ig::BoxLayout &phongParamLayout;
        ui::ig::ColorEdit<glm::vec3> &ambientColPicker;
        ui::ig::ColorEdit<glm::vec3> &diffuseColPicker;
        ui::ig::ColorEdit<glm::vec3> &specularColPicker;
  ui::ig::Window &debugWindow;
    ui::ig::TabBar &debugTabBar;
      ui::ig::Tab &logTab;
        ui::ig::Memo &logMemo;
        ui::ig::Memo &logErrMemo;
      ui::ig::Tab &chaiscriptTab;
        ui::ig::BoxLayout &chaiInputLayout;
          ui::ig::Text &chaiInputLabel;
          ui::ig::InputText &chaiInputText;
        ui::ig::Memo &chaiOutputMemo;
        ui::ig::Button &chaiConfirmButton;
  ui::ig::Window &infoWindow;
    ui::ig::SimplePlot &fpsCurrentPlot;
    ui::ig::SimplePlot &fpsAveragePlot;
    ui::ig::Text &fpsLabel;
    ui::ig::Button &resetFpsButton;
    ui::ig::Checkbox &vsyncCheckbox;
    ui::ig::FlameGraph &flameGraph;
    ui::ig::Group &cameraGroup;
      ui::ig::Text &cameraPosText;
      ui::ig::Text &cameraDirText;
      ui::ig::Button &cameraToOriginButton;
      ui::ig::Slider<float> &cameraMoveSpeedSlider;
      ui::ig::Slider<float> &cameraMouseSpeedSlider;
      ui::ig::Slider<int> &cameraFOVSlider;
  ui::ig::Group &sceneGroup;
    ui::ig::Text &sceneModelCountText;
    ui::ig::Text &sceneVoxelCountText;
    ui::ig::Text &sceneBVHNodeCountText;
    ui::ig::Text &sceneBVHDepthText;
  ui::ig::Window &debugImagesWindow;
    ui::ig::StretchLayout &imageStretchLayout;
      ui::ig::Image &iterationImage;
  ui::ig::Window &shaderControlsWindow;
    ui::ig::Checkbox &debugPrintEnableCheckbox;
    ui::ig::Checkbox &bvhVisualizeCheckbox;
    ui::ig::Checkbox &visualizeProbesCheckbox;
    ui::ig::SpinInput<int> &shaderDebugValueInput;
    ui::ig::DragInput<float> &shaderDebugFloatValueSlider;
    ui::ig::DragInput<float> &shaderDebugIterDivideDrag;
  ui::ig::Window &modelsWindow;
    ui::ig::Text &modelLoadingSettingsTitle;
    ui::ig::BoxLayout &modelLoadingSettings;
      ui::ig::Checkbox &modelLoadingSeparateModelsCheckbox;
    ui::ig::AbsoluteLayout &modelListsLayout;
      ui::ig::Listbox<ModelFileInfo> &modelList;
      ui::ig::InputText &modelsFilterInput;
      ui::ig::Button &reloadModelListButton;
      ui::ig::Button &activateSelectedModelButton;
      ui::ig::Listbox<ModelFileInfo> &activeModelList;
      ui::ig::BoxLayout &activeModelsLayout;
        ui::ig::Button &clearActiveModelsButton;
        ui::ig::InputText &activeModelsFilterInput;
    ui::ig::Text &modelDetailTitle;
    ui::ig::BoxLayout &modelDetailLayout;
      ui::ig::InputText &modelDetailIIDText;
      ui::ig::InputText &modelDetailPathText;
      ui::ig::InputText &modelDetailSVOHeightText;
      ui::ig::InputText &modelDetailVoxelCountText;
      ui::ig::InputText &modelDetailMinimisedVoxelCountText;
      ui::ig::Separator &modelDetailSeparator1;
      ui::ig::Text &modelDetailBufferInfo;
      ui::ig::Text &modelDetailBufferOffset;
      ui::ig::Text &modelDetailBufferSize;
      ui::ig::DragInput<glm::vec3> &modelDetailTranslateDrag;
      ui::ig::DragInput<glm::vec3> &modelDetailRotateDrag;
      ui::ig::DragInput<glm::vec3> &modelDetailScaleDrag;

  // clang-format on

  void setWindowsVisible(bool visible);

  void createConvertWindow(ThreadPool &threadPool, const std::filesystem::path &modelFolder,
                           std::invocable<std::filesystem::path, std::filesystem::path> auto &&conversion) {
    using namespace ui::ig;
    if (imgui->windowByName("convert_svo_window").has_value()) { return; }
    auto &convertWindow = imgui->createWindow("convert_svo_window", "Convert svo to binary");
    convertWindow.setCloseable(true);
    convertWindow.setSize(Size{500, 400});
    auto &selectFilesButton = convertWindow.createChild<Button>("convert_svo_select_files_btn", "Select input files");
    auto &selectedFilesLayout = convertWindow.createChild<BoxLayout>(
        "convert_svo_select_files_layout", LayoutDirection::TopToBottom, Size{Width::Auto(), 200});
    selectedFilesLayout.setScrollable(true);
    selectedFilesLayout.setDrawBorder(true);
    auto &selectedFilesText = selectedFilesLayout.createChild<Text>("convert_svo_select_files_list", "Selected files:");
    auto &selectOutputAndConvertButton =
        convertWindow.createChild<Button>("convert_svo_select_output_btn", "Select output folder and convert");

    selectFilesButton.addClickListener([&, modelFolder] {
      imgui->openFileDialog(
          "Select files to convert", {FileExtensionSettings{{"vox"}, "Vox model", ImVec4{1, 0, 0, 1}}},
          [&](const std::vector<std::filesystem::path> &selectedFiles) {
            constexpr auto SELECTED_FILES_TEMPLATE = "Selected files:\n{}";
            selectedFilesText.setText(
                SELECTED_FILES_TEMPLATE,
                selectedFiles | ranges::views::transform([](const auto &path) { return path.string(); })
                    | ranges::views::cache1 | ranges::views::join('\n') | ranges::to<std::string>());
            filesToConvert = selectedFiles;
          },
          [] {}, Size{500, 400}, modelFolder, "", Modal::Yes, 0);
    });

    selectOutputAndConvertButton.addClickListener([&, modelFolder, conversion] {
      imgui->openDirDialog(
          "Select output dir",
          [&](const auto selectedDirs) {
            const auto selectedDir = selectedDirs[0];
            const auto &[loadingDialog, loadingProgress, loadingText] = createLoadingDialog();
            loadingProgress.setMin(0);
            loadingProgress.setMax(filesToConvert.size());
            std::ranges::for_each(filesToConvert, [&, selectedDir](const auto &file) {
              threadPool.enqueue([&, selectedDir] {
                conversion(file, selectedDir);
                window->enqueue([&] {
                  loadingProgress.step();
                  if (loadingProgress.getValue() == filesToConvert.size()) { loadingDialog.close(); }
                });
              });
            });
          },
          [] {}, Size{500, 400}, modelFolder);
    });

    convertWindow.addCloseListener([&] {
      filesToConvert.clear();
      window->enqueue([&] { imgui->removeWindow(convertWindow.getName()); });
    });
  }

  std::tuple<ui::ig::ModalDialog &, ui::ig::ProgressBar<float> &, ui::ig::Text &> createLoadingDialog();

  constexpr static auto MODEL_BUFFER_OFFSET_INFO = "Offset: {} B";
  constexpr static auto MODEL_BUFFER_SIZE_INFO = "Size: {} B";
  constexpr static auto SCENE_VOXEL_COUNT_INFO = "Voxel count: {} voxels";
  constexpr static auto SCENE_MODEL_COUNT_INFO = "Model count: {} models";
  constexpr static auto SCENE_BVH_NODE_COUNT_INFO = "BVH node count: {} node";
  constexpr static auto SCENE_BVH_DEPTH_INFO = "BVH depth: {} levels";

 private:
  std::vector<std::filesystem::path> filesToConvert{};
};

}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_UI_SIMPLESVORENDERER_UI_H
