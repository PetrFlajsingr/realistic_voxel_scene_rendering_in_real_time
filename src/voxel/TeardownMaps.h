//
// Created by petr on 6/17/21.
//

#ifndef HIHI__TEARDOWNMAPS_H
#define HIHI__TEARDOWNMAPS_H

#include <algorithm>
#include <filesystem>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <tinyxml2.h>
#include <variant>
#include <vector>

#include "RawVoxelModel.h"
#include "RawVoxelScene.h"

// TODO: move this to pf_common
inline std::vector<std::string> split(std::string_view str, std::string_view delimiter) {
  auto result = std::vector<std::string>{};
  size_t first = 0;
  while (first < str.size()) {
    const auto second = str.find_first_of(delimiter, first);
    if (first != second) { result.emplace_back(str.substr(first, second - first)); }
    if (second == std::string_view::npos) { break; }
    first = second + 1;
  }
  return result;
}
// TODO: move this to pf_common
inline std::string replace(std::string subject, const std::string &search, const std::string &replace) {
  std::size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
  return subject;
}

namespace TeardownMap {
struct Group;
template<typename T>
auto makeFromXml(const std::filesystem::path &sceneFolder) {
  return [sceneFolder](tinyxml2::XMLElement *el) { return T::FromXml(el, sceneFolder); };
}

template<typename T>
std::vector<T> eachChildFromXml(tinyxml2::XMLElement *el, const std::string &name,
                                [[maybe_unused]] const std::filesystem::path &sceneFolder) {
  auto result = std::vector<T>{};
  for (auto groupIter = el->FirstChildElement(name.c_str()); groupIter != nullptr;
       groupIter = groupIter->NextSiblingElement(name.c_str())) {
    result.emplace_back(makeFromXml<T>(sceneFolder)(groupIter));
  }
  return result;
}

glm::vec3 xmlStrToGlmVec3(std::string_view str);

glm::vec4 xmlStrToGlmVec4(std::string_view str);

std::string strAttribOr(tinyxml2::XMLElement *el, const std::string &name, std::string_view def);

// Vox, VoxBox, vehicle, Wheel
struct VoxData {
  glm::vec3 position;
  glm::vec3 rotation;
  float scale;
  bool hidden;
  std::filesystem::path file;
  std::string objectName;
  std::string origTag;

  void loadRawVoxelData(std::unordered_map<std::string, std::unique_ptr<pf::vox::RawVoxelScene>> &fileCache);
  std::variant<std::unique_ptr<pf::vox::RawVoxelModel>, std::unique_ptr<pf::vox::RawVoxelScene>> rawVoxelData;
};

// Group, Instance, VoxBox, Body
struct VoxDataGroup {
  glm::vec3 position;
  glm::vec3 rotation;
  float scale;
  std::vector<VoxDataGroup> groups;
  std::vector<VoxData> voxData;

  void loadRawVoxelData(std::unordered_map<std::string, std::unique_ptr<pf::vox::RawVoxelScene>> &fileCache);
};

struct Water {
  float depth;
  glm::vec3 position;

  static Water FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Environment {
  std::string name;
  glm::vec3 skyboxRotation;
  glm::vec3 position;
  bool raining;
  glm::vec3 skyboxTint;
  float skyboxBrightness;
  float ambient;
  float brightness;

  static Environment FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Spawnpoint {
  std::string name;
  glm::vec3 rotation;
  glm::vec3 position;

  static Spawnpoint FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Location {
  glm::vec3 rotation;
  glm::vec3 position;

  static Location FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Light {
  float angle;
  std::string type;
  glm::vec3 rotation;
  glm::vec3 position;
  glm::vec3 color;
  glm::vec3 size;
  float scale;

  static Light FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Vox {
  std::string name;
  std::vector<Location> locations;
  std::vector<Vox> voxes;
  std::vector<Light> lights;

  float scale;
  std::string texture;//?
  glm::vec3 rotation;
  std::filesystem::path voxFile;
  std::string objectNameInFile;
  bool hidden;

  VoxDataGroup toVoxDataGroup();

  static Vox FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Instance {
  glm::vec3 rotation;
  glm::vec3 position;
  std::filesystem::path xmlFile;
  std::vector<Group> groups;

  VoxDataGroup toVoxDataGroup();

  static Instance FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct VoxBox {
  std::vector<Instance> instances;
  std::vector<Light> lights;
  std::vector<Vox> voxes;

  std::string texture;//?
  glm::vec3 size;
  glm::vec3 position;
  glm::vec3 rotation;
  std::string material;
  std::filesystem::path brushFile;
  glm::vec3 color;
  std::string objectNameInFile;
  glm::vec3 offset;
  glm::vec4 pbr;

  VoxDataGroup toVoxDataGroup();

  static VoxBox FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Wheel {
  std::string name;
  std::vector<Vox> voxes;
  glm::vec3 position;

  VoxDataGroup toVoxDataGroup();

  static Wheel FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Body {
  std::vector<Body> bodies;
  std::vector<Group> groups;
  std::vector<VoxBox> voxBoxes;
  std::vector<Instance> instances;
  std::vector<Vox> voxes;
  std::vector<Wheel> wheels;

  std::string name;
  glm::vec3 rotation;
  glm::vec3 position;

  VoxDataGroup toVoxDataGroup();

  static Body FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Vehicle;

struct Group {
  std::string name;
  glm::vec3 position;
  glm::vec3 rotation;

  std::vector<Water> waters;
  std::vector<Environment> environments;
  std::vector<Spawnpoint> spawnPoints;
  std::vector<Group> groups;
  std::vector<Vox> voxes;
  std::vector<VoxBox> voxBoxes;
  std::vector<Body> bodies;
  std::vector<Instance> instances;
  std::vector<Vehicle> vehicles;

  VoxDataGroup toVoxDataGroup();

  static Group FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

// TODO: generate terrain based on height map
struct VoxScript {

  static VoxScript FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Vehicle {
  std::vector<Body> bodies;
  std::string name;
  glm::vec3 position;
  glm::vec3 rotation;

  VoxDataGroup toVoxDataGroup();

  static Vehicle FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

struct Scene {
  std::vector<Instance> instances;
  std::vector<VoxScript> voxScripts;
  std::vector<Vehicle> vehicles;
  std::vector<Group> groups;

  VoxDataGroup toVoxDataGroup();

  static Scene FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder);
};

}// namespace TeardownMap

#endif//HIHI__TEARDOWNMAPS_H
