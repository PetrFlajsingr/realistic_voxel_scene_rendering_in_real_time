//
// Created by petr on 11/2/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UTILS_CONFIG_H
#define REALISTIC_VOXEL_RENDERING_UTILS_CONFIG_H

#include <toml++/toml.h>
#include <filesystem>

using TomlConfig = decltype(toml::parse_file(std::declval<std::string>()));
using SharedTomlConfig = std::shared_ptr<TomlConfig>;

#endif//REALISTIC_VOXEL_RENDERING_UTILS_CONFIG_H
