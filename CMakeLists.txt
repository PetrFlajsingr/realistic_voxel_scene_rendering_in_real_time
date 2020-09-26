cmake_minimum_required(VERSION 3.17)
include("cmake/HunterGate.cmake")
include("cmake/CPM.cmake")
include(FetchContent)

function(print_all_vars)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.269.tar.gz"
        SHA1 "64024b7b95b4c86d50ae05b926814448c93a70a0"
)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()


project(
        realistic_voxel_scene_rendering_in_real_time
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts -fconcepts-diagnostics-depth=10")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -fcoroutines")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(VK_LAYER_PATH "/etc/vulkan/explicit_layer.d")
set(LD_LIBRARY_PATH "/usr/lib")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

hunter_add_package(Microsoft.GSL)
find_package(Microsoft.GSL CONFIG REQUIRED)
hunter_add_package(imgui)
find_package(imgui CONFIG REQUIRED)
hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)
hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)
add_compile_definitions(SPDLOG_FMT_EXTERNAL)

CPMAddPackage(
        NAME magic_enum
        GITHUB_REPOSITORY Neargye/magic_enum
        GIT_TAG v0.6.6
)
CPMAddPackage(
        NAME units
        GITHUB_REPOSITORY nholthaus/units
        GIT_TAG v2.3.1
)
CPMAddPackage(
        NAME backward_cpp
        GITHUB_REPOSITORY bombela/backward-cpp
        GIT_TAG v1.5
)
CPMAddPackage(
        NAME range-v3
        GITHUB_REPOSITORY ericniebler/range-v3
        GIT_TAG 0.11.0
)
CPMAddPackage(
        NAME nanobench
        GITHUB_REPOSITORY martinus/nanobench
        GIT_TAG v4.2.0
)
CPMAddPackage(
        NAME subprocess
        GITHUB_REPOSITORY benman64/subprocess
        GIT_TAG v0.4.0
)
CPMAddPackage(
        NAME argparse
        GITHUB_REPOSITORY p-ranav/argparse
        GIT_TAG v2.1
)
CPMAddPackage(
        NAME toml11
        GITHUB_REPOSITORY ToruNiina/toml11
        GIT_TAG v3.6.0
)
CPMAddPackage(
        NAME chaiscript
        GITHUB_REPOSITORY ChaiScript/ChaiScript
        GIT_TAG v6.1.0
)
if (chaiscript_ADDED)
    add_library(chaiscript INTERFACE IMPORTED)
    target_include_directories(chaiscript INTERFACE ${chaiscript_SOURCE_DIR}/include)
endif()
include_directories(${CMAKE_SOURCE_DIR}/third_party/)

add_compile_definitions(GLFW_INCLUDE_VULKAN
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL
        GLM_FORCE_SWIZZLE
        VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE)


#set(IMPLOT_CPP_FILES third_party/implot/implot.cpp third_party/implot/implot_items.cpp)
add_executable(realistic_voxel_scene_rendering_in_real_time main.cpp ${BACKWARD_ENABLE}
        exceptions/stacktrace_exception.cpp exceptions/stacktrace_exception.h
        args/valid_path_check_action.cpp
        #${IMPLOT_CPP_FILES}
        utils.h logging/loggers.cpp logging/loggers.h logging/some_levels_sink.h args/regex_check_action.h args/regex_check_action.cpp application.h concepts/window.cpp ui/glfw_window.cpp ui/glfw_window.h ui/events/common.h ui/events/subscription.cpp ui/events/subscription.h concepts/iterable.h concepts/streams.h coroutines/sequence.h concepts/incrementable.h ui/events/event_dispatch_impl.h vulkan/vulkan_interface.h vulkan/vulkan_exception.cpp vulkan/vulkan_exception.h vulkan/utils.h vulkan/utils.cpp vulkan/default_device_suitability_scorer.cpp vulkan/default_device_suitability_scorer.h)
target_link_libraries(realistic_voxel_scene_rendering_in_real_time
        -lbfd -ldl
        magic_enum imgui::imgui units fmt::fmt range-v3 nanobench subprocess argparse toml11 chaiscript
        ${GLFW_STATIC_LIBRARIES} ${GLM_LIBRARIES} ${Vulkan_LIBRARIES})
add_backward(realistic_voxel_scene_rendering_in_real_time)


