//
// Created by petr on 10/18/20.
//

#ifndef VOXEL_RENDER_COMPILER_H
#define VOXEL_RENDER_COMPILER_H

#include <pf_common/exceptions/StackTraceException.h>
#include <shaderc/shaderc.hpp>
#include <string>
#include <utility>

namespace pf::glsl {

using MacroDefs = std::vector<std::string>;
using ReplaceMacroDefs = std::vector<std::pair<std::string, std::string>>;
using BinaryData = std::vector<uint32_t>;
enum class CompilationStep {
  None, Preprocessed, Assembly, Binary
};

enum class Optimization {
  None, Size, Performance
};

class Compiler {
 public:
  Compiler(std::string srcName, std::string src, shaderc_shader_kind type, const MacroDefs& macros = {},
           const ReplaceMacroDefs& replaceMacros = {});

  std::string preprocess();
  [[nodiscard]] std::string toAssembly(Optimization optimization = Optimization::None);
  [[nodiscard]] BinaryData toBinary(Optimization optimization = Optimization::None);
  [[nodiscard]] BinaryData compile(Optimization optimization = Optimization::None);

 private:
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  std::string name;
  std::string source;
  shaderc_shader_kind kind;
  MacroDefs macros;
  CompilationStep currentStep = CompilationStep::None;
  BinaryData binaryData;
};

class CompilationException : public StackTraceException {
 public:
  explicit CompilationException(const std::string_view &message);
};

}// namespace pf::glsl
#endif//VOXEL_RENDER_COMPILER_H
