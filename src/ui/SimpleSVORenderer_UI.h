//
// Created by petr on 5/21/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UI_SIMPLESVORENDERER_UI_H
#define REALISTIC_VOXEL_RENDERING_SRC_UI_SIMPLESVORENDERER_UI_H

#include <glm/glm.hpp>
#include <ostream>
#include <pf_common/enums.h>
#include <pf_common/math/BoundingBox.h>
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
  ModelFileInfo() = default;
  ModelFileInfo(const std::filesystem::path &path);
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
  explicit SimpleSVORenderer_UI(std::unique_ptr<ui::ig::ImGuiGlfwVulkan> &&imguiInterface, const Camera &camera,
                                TextureData iterTextureData);

  std::unique_ptr<ui::ig::ImGuiGlfwVulkan> imgui;

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
      ui::ig::Slider<float> &cameraMoveSpeedSlider;
      ui::ig::Slider<float> &cameraMouseSpeedSlider;
      ui::ig::Slider<int> &cameraFOVSlider;
  ui::ig::Window &debugImagesWindow;
    ui::ig::StretchLayout &imageStretchLayout;
      ui::ig::Image &iterationImage;
  ui::ig::Window &shaderControlsWindow;
    ui::ig::Checkbox &debugPrintEnableCheckbox;
    ui::ig::Checkbox &bvhVisualizeCheckbox;
    ui::ig::SpinInput<int> &shaderDebugValueInput;
    ui::ig::DragInput<float> &shaderDebugFloatValueSlider;
    ui::ig::DragInput<float> &shaderDebugIterDivideDrag;
  ui::ig::Window &modelsWindow;
    ui::ig::AbsoluteLayout &modelListsLayout;
      ui::ig::Listbox<ModelFileInfo> &modelList;
      ui::ig::InputText &modelsFilterInput;
      ui::ig::Button &reloadModelListButton;
      ui::ig::Button &activateSelectedModelButton;
      ui::ig::Listbox<ModelFileInfo> &activeModelList;
      ui::ig::BoxLayout &activeModelsLayout;
        ui::ig::Button &removeSelectedActiveModelButton;
        ui::ig::Button &createInstanceSelectedActiveModelButton;
        ui::ig::Button &duplicateSelectedActiveModelButton;
    ui::ig::Text &modelDetailTitle;
    ui::ig::BoxLayout &modelDetailLayout;
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

  constexpr static auto MODEL_BUFFER_OFFSET_INFO = "Offset: {} B";
  constexpr static auto MODEL_BUFFER_SIZE_INFO = "Size: {} B";

 private:
};

}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_UI_SIMPLESVORENDERER_UI_H
