//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_INPUTTEXT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_INPUTTEXT_H

#include "Text.h"
#include "interface/LabeledElement.h"
#include "interface/ValueObservableElement.h"
#include <functional>
#include <imgui.h>
namespace pf::ui::ig {

enum class TextInputType { SingleLine, MultiLine };

class InputText : public Text,
                       public LabeledElement,
                       public ValueObservableElement<std::string_view> {
 public:
  InputText(const std::string &elementName, std::string caption, const std::string &text = "",
                 TextInputType textInputType = TextInputType::SingleLine);

  void clear();

 protected:
  void renderImpl() override;

 private:
  char buffer[256]{};
  TextInputType inputType;
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_INPUTTEXT_H
