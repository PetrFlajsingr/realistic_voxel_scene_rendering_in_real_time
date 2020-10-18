//
// Created by petr on 10/18/20.
//

#include "Compiler.h"

namespace pf::glsl {

Compiler::Compiler(std::string srcName, std::string src, shaderc_shader_kind type,
                   const MacroDefs &macros, const ReplaceMacroDefs &replaceMacros)
    : name(std::move(srcName)), source(std::move(src)), kind(type) {
  for (const auto &macro : macros) { options.AddMacroDefinition(macro); }
  for (const auto &[macro, value] : replaceMacros) { options.AddMacroDefinition(macro, value); }
}

std::string Compiler::preprocess() {
  if (currentStep == CompilationStep::None) {
    const auto result = compiler.PreprocessGlsl(source, kind, name.c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
      throw CompilationException(result.GetErrorMessage());
    }
    source = {result.cbegin(), result.cend()};
    currentStep = CompilationStep::Preprocessed;
  } else if (currentStep != CompilationStep::Preprocessed) {
    throw CompilationException("Compiler is beyond preprocess state");
  }

  return source;
}
std::string Compiler::toAssembly(Optimization optimization) {
  if (currentStep == CompilationStep::Preprocessed) {
    switch (optimization) {
      case Optimization::None: options.SetOptimizationLevel(shaderc_optimization_level_zero); break;
      case Optimization::Size: options.SetOptimizationLevel(shaderc_optimization_level_size); break;
      case Optimization::Performance:
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        break;
    }
    auto result = compiler.CompileGlslToSpvAssembly(source, kind, name.c_str(), options);
    source = {result.cbegin(), result.cend()};
    currentStep = CompilationStep::Assembly;
  } else if (currentStep != CompilationStep::Assembly) {
    throw CompilationException("Compiler is beyond assembly state");
  }
  return source;
}
BinaryData Compiler::toBinary(Optimization optimization) {
  if (currentStep == CompilationStep::Preprocessed) {
    switch (optimization) {
      case Optimization::None: options.SetOptimizationLevel(shaderc_optimization_level_zero); break;
      case Optimization::Size: options.SetOptimizationLevel(shaderc_optimization_level_size); break;
      case Optimization::Performance:
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        break;
    }
    auto result = compiler.CompileGlslToSpvAssembly(source, kind, name.c_str(), options);
    binaryData = {result.cbegin(), result.cend()};
    currentStep = CompilationStep::Binary;
  } else if (currentStep != CompilationStep::Binary) {
    throw CompilationException("Compiler is beyond assembly state");
  }
  return binaryData;
}
BinaryData Compiler::compile(Optimization optimization) {
  if (currentStep == CompilationStep::None) {
    preprocess();
  }
  if (currentStep == CompilationStep::Preprocessed) {
    return toBinary(optimization);
  } else if (currentStep != CompilationStep::Binary) {
    throw CompilationException("Compiler is beyond binary state");
  }
  return binaryData;
}

CompilationException::CompilationException(const std::string_view &message)
    : StackTraceException(message) {}
}// namespace pf::glsl