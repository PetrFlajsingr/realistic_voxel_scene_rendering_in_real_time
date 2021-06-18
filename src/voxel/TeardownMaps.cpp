//
// Created by petr on 6/17/21.
//

#include "TeardownMaps.h"
#include "ModelLoading.h"
#include <logging/loggers.h>
#include <unordered_map>

namespace TeardownMap {
glm::vec3 xmlStrToGlmVec3(std::string_view str) {
  auto splitStr = split(str, " ");
  if (splitStr.size() < 3) { splitStr.emplace_back("0"); }
  if (splitStr.size() < 3) { splitStr.emplace_back("0"); }
  return glm::vec3{std::stof(std::string{splitStr[0]}), std::stof(std::string{splitStr[1]}),
                   std::stof(std::string{splitStr[2]})};
}

std::string strAttribOr(tinyxml2::XMLElement *el, const std::string &name, std::string_view def) {
  auto r = el->Attribute(name.c_str());
  if (r == nullptr) { return std::string{def}; }
  return r;
}
glm::vec4 xmlStrToGlmVec4(std::string_view str) {
  auto splitStr = split(str, " ");

  if (splitStr.size() < 4) { splitStr.emplace_back("0"); }
  if (splitStr.size() < 4) { splitStr.emplace_back("0"); }
  if (splitStr.size() < 4) { splitStr.emplace_back("0"); }
  return glm::vec4{std::stof(std::string{splitStr[0]}), std::stof(std::string{splitStr[1]}),
                   std::stof(std::string{splitStr[2]}), std::stof(std::string{splitStr[3]})};
}

VoxScript VoxScript::FromXml([[maybe_unused]] tinyxml2::XMLElement *element,
                             [[maybe_unused]] const std::filesystem::path &sceneFolder) {
  return VoxScript{};
}
VoxDataGroup Vehicle::toVoxDataGroup() {
  auto result = VoxDataGroup{};
  result.position = position;
  result.rotation = rotation;
  result.scale = 1;
  std::ranges::transform(bodies, std::back_inserter(result.groups), [](Body &body) { return body.toVoxDataGroup(); });
  return result;
}
Vehicle Vehicle::FromXml(tinyxml2::XMLElement *element, const std::filesystem::path &sceneFolder) {
  auto result = Vehicle{};
  result.bodies = eachChildFromXml<Body>(element, "body", sceneFolder);
  result.name = strAttribOr(element, "name", "");
  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  result.rotation = xmlStrToGlmVec3(strAttribOr(element, "rot", "0 0 0"));
  return result;
}
VoxDataGroup Scene::toVoxDataGroup() {
  auto result = VoxDataGroup{};
  result.position = glm::vec3{0};
  result.rotation = glm::vec3{0};
  result.scale = 1;
  std::ranges::transform(instances, std::back_inserter(result.groups),
                         [](Instance &instance) { return instance.toVoxDataGroup(); });
  std::ranges::transform(groups, std::back_inserter(result.groups),
                         [](Group &group) { return group.toVoxDataGroup(); });
  std::ranges::transform(vehicles, std::back_inserter(result.groups),
                         [](Vehicle &vehicle) { return vehicle.toVoxDataGroup(); });
  return result;
}
Scene Scene::FromXml(tinyxml2::XMLElement *element, const std::filesystem::path &sceneFolder) {
  auto result = Scene{};
  result.groups = eachChildFromXml<Group>(element, "group", sceneFolder);
  result.vehicles = eachChildFromXml<Vehicle>(element, "vehicle", sceneFolder);
  result.voxScripts = eachChildFromXml<VoxScript>(element, "voxscript", sceneFolder);
  result.instances = eachChildFromXml<Instance>(element, "instance", sceneFolder);
  return result;
}
Group Group::FromXml(tinyxml2::XMLElement *element, const std::filesystem::path &sceneFolder) {
  auto result = Group{};
  result.waters = eachChildFromXml<Water>(element, "water", sceneFolder);
  result.environments = eachChildFromXml<Environment>(element, "environment", sceneFolder);
  result.spawnPoints = eachChildFromXml<Spawnpoint>(element, "spawnpoint", sceneFolder);
  result.groups = eachChildFromXml<Group>(element, "group", sceneFolder);
  result.voxes = eachChildFromXml<Vox>(element, "vox", sceneFolder);
  result.voxBoxes = eachChildFromXml<VoxBox>(element, "voxbox", sceneFolder);
  result.bodies = eachChildFromXml<Body>(element, "body", sceneFolder);
  result.instances = eachChildFromXml<Instance>(element, "instance", sceneFolder);
  result.vehicles = eachChildFromXml<Vehicle>(element, "vehicle", sceneFolder);

  result.name = strAttribOr(element, "name", "");
  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  result.rotation = xmlStrToGlmVec3(strAttribOr(element, "rot", "0 0 0"));
  return result;
}
VoxDataGroup Group::toVoxDataGroup() {
  auto result = VoxDataGroup{};
  result.position = position;
  result.rotation = rotation;
  result.scale = 1;
  std::ranges::transform(voxes, std::back_inserter(result.groups), [](Vox &vox) { return vox.toVoxDataGroup(); });
  std::ranges::transform(instances, std::back_inserter(result.groups),
                         [](Instance &instance) { return instance.toVoxDataGroup(); });
  std::ranges::transform(groups, std::back_inserter(result.groups),
                         [](Group &group) { return group.toVoxDataGroup(); });
  std::ranges::transform(bodies, std::back_inserter(result.groups), [](Body &body) { return body.toVoxDataGroup(); });
  std::ranges::transform(voxBoxes, std::back_inserter(result.groups),
                         [](VoxBox &voxBox) { return voxBox.toVoxDataGroup(); });
  std::ranges::transform(vehicles, std::back_inserter(result.groups),
                         [](Vehicle &vehicle) { return vehicle.toVoxDataGroup(); });
  return result;
}
Body Body::FromXml(tinyxml2::XMLElement *element, const std::filesystem::path &sceneFolder) {
  auto result = Body{};
  result.bodies = eachChildFromXml<Body>(element, "body", sceneFolder);
  result.groups = eachChildFromXml<Group>(element, "group", sceneFolder);
  result.voxBoxes = eachChildFromXml<VoxBox>(element, "voxbox", sceneFolder);
  result.instances = eachChildFromXml<Instance>(element, "instance", sceneFolder);
  result.voxes = eachChildFromXml<Vox>(element, "vox", sceneFolder);
  result.wheels = eachChildFromXml<Wheel>(element, "wheel", sceneFolder);

  result.name = strAttribOr(element, "name", "");
  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  result.rotation = xmlStrToGlmVec3(strAttribOr(element, "rot", "0 0 0"));
  return result;
}
VoxDataGroup Body::toVoxDataGroup() {
  auto result = VoxDataGroup{};
  result.position = position;
  result.rotation = rotation;
  result.scale = 1;
  std::ranges::transform(voxes, std::back_inserter(result.groups), [](Vox &vox) { return vox.toVoxDataGroup(); });
  std::ranges::transform(instances, std::back_inserter(result.groups),
                         [](Instance &instance) { return instance.toVoxDataGroup(); });
  std::ranges::transform(groups, std::back_inserter(result.groups),
                         [](Group &group) { return group.toVoxDataGroup(); });
  std::ranges::transform(bodies, std::back_inserter(result.groups), [](Body &body) { return body.toVoxDataGroup(); });
  std::ranges::transform(wheels, std::back_inserter(result.groups),
                         [](Wheel &wheel) { return wheel.toVoxDataGroup(); });
  return result;
}
Wheel Wheel::FromXml(tinyxml2::XMLElement *element, const std::filesystem::path &sceneFolder) {
  auto result = Wheel{};
  result.voxes = eachChildFromXml<Vox>(element, "vox", sceneFolder);

  result.name = strAttribOr(element, "name", "");
  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  return result;
}
VoxDataGroup Wheel::toVoxDataGroup() {
  auto result = VoxDataGroup{};
  result.position = position;
  result.rotation = glm::vec3{0};
  result.scale = 1;
  std::ranges::transform(voxes, std::back_inserter(result.groups), [](Vox &vox) { return vox.toVoxDataGroup(); });
  return result;
}
VoxBox VoxBox::FromXml(tinyxml2::XMLElement *element, const std::filesystem::path &sceneFolder) {
  auto result = VoxBox{};
  result.instances = eachChildFromXml<Instance>(element, "instance", sceneFolder);
  result.lights = eachChildFromXml<Light>(element, "light", sceneFolder);
  result.voxes = eachChildFromXml<Vox>(element, "vox", sceneFolder);

  result.size = xmlStrToGlmVec3(strAttribOr(element, "size", "0 0 0"));
  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  result.rotation = xmlStrToGlmVec3(strAttribOr(element, "rot", "0 0 0"));
  result.color = xmlStrToGlmVec3(strAttribOr(element, "color", "0 0 0"));
  result.offset = xmlStrToGlmVec3(strAttribOr(element, "offset", "0 0 0"));
  result.texture = strAttribOr(element, "texture", "");
  result.material = strAttribOr(element, "material", "");
  result.brushFile = replace(strAttribOr(element, "brush", ""), "MOD", sceneFolder.string());
  result.objectNameInFile = strAttribOr(element, "object", "");
  result.pbr = xmlStrToGlmVec4(strAttribOr(element, "pbr", "0 0 0 0"));
  return result;
}
VoxDataGroup VoxBox::toVoxDataGroup() {
  auto result = VoxDataGroup{};
  result.position = position;
  result.rotation = rotation;
  result.scale = 1;
  std::ranges::transform(voxes, std::back_inserter(result.groups), [](Vox &vox) { return vox.toVoxDataGroup(); });
  std::ranges::transform(instances, std::back_inserter(result.groups),
                         [](Instance &instance) { return instance.toVoxDataGroup(); });
#if false // todo: fix this
  VoxData voxData{};
  voxData.position = glm::vec3{0};
  voxData.rotation = glm::vec3{0};
  voxData.hidden = false;
  voxData.objectName = objectNameInFile;
  voxData.file = brushFile;
  voxData.scale = 1;
  voxData.origTag = std::string("voxbox: ") ;
  result.voxData.emplace_back(std::move(voxData));
#endif
  return result;
}
Instance Instance::FromXml(tinyxml2::XMLElement *element, const std::filesystem::path &sceneFolder) {
  auto result = Instance{};

  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  result.rotation = xmlStrToGlmVec3(strAttribOr(element, "rot", "0 0 0"));
  result.xmlFile = replace(strAttribOr(element, "file", ""), "MOD", sceneFolder.string());
  tinyxml2::XMLDocument doc;
  doc.LoadFile(result.xmlFile.string().c_str());
  result.groups = eachChildFromXml<Group>(doc.RootElement(), "group", sceneFolder);

  return result;
}
VoxDataGroup Instance::toVoxDataGroup() {
  auto result = VoxDataGroup{};
  result.position = position;
  result.rotation = rotation;
  result.scale = 1;
  std::ranges::transform(groups, std::back_inserter(result.groups),
                         [](Group &group) { return group.toVoxDataGroup(); });
  return result;
}
Vox Vox::FromXml(tinyxml2::XMLElement *element, const std::filesystem::path &sceneFolder) {
  auto result = Vox{};
  result.locations = eachChildFromXml<Location>(element, "location", sceneFolder);
  result.lights = eachChildFromXml<Light>(element, "light", sceneFolder);
  result.voxes = eachChildFromXml<Vox>(element, "vox", sceneFolder);

  result.name = strAttribOr(element, "name", "");
  result.scale = element->FloatAttribute("scale", 1.f);
  result.rotation = xmlStrToGlmVec3(strAttribOr(element, "rot", "0 0 0"));
  result.texture = strAttribOr(element, "texture", "");
  result.objectNameInFile = strAttribOr(element, "object", "");
  result.voxFile = replace(strAttribOr(element, "file", ""), "MOD", sceneFolder.string());
  result.hidden = element->BoolAttribute("hidden", false);
  return result;
}
VoxDataGroup Vox::toVoxDataGroup() {
  auto result = VoxDataGroup{};
  result.position = glm::vec3{0};
  result.rotation = rotation;
  result.scale = scale;
  std::ranges::transform(voxes, std::back_inserter(result.groups), [](Vox &vox) { return vox.toVoxDataGroup(); });
  VoxData voxData{};
  voxData.position = glm::vec3{0};
  voxData.rotation = glm::vec3{0};
  voxData.hidden = hidden;
  voxData.objectName = objectNameInFile;
  voxData.file = voxFile;
  voxData.scale = 1;
  voxData.origTag = std::string("vox: ") + name;
  result.voxData.emplace_back(std::move(voxData));
  return result;
}
Light Light::FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder) {
  auto result = Light{};

  result.angle = element->FloatAttribute("angle", 0);
  result.type = strAttribOr(element, "type", "");
  result.rotation = xmlStrToGlmVec3(strAttribOr(element, "rot", "0 0 0"));
  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  result.color = xmlStrToGlmVec3(strAttribOr(element, "color", "0 0 0"));
  result.size = xmlStrToGlmVec3(strAttribOr(element, "size", "0 0 0"));
  result.scale = element->FloatAttribute("scale", 1);
  return result;
}
Location Location::FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder) {
  auto result = Location{};

  result.rotation = xmlStrToGlmVec3(strAttribOr(element, "rot", "0 0 0"));
  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  return result;
}
Spawnpoint Spawnpoint::FromXml(tinyxml2::XMLElement *element,
                               [[maybe_unused]] const std::filesystem::path &sceneFolder) {
  auto result = Spawnpoint{};

  result.name = strAttribOr(element, "name", "");
  result.rotation = xmlStrToGlmVec3(strAttribOr(element, "rot", "0 0 0"));
  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  return result;
}
Environment Environment::FromXml(tinyxml2::XMLElement *element,
                                 [[maybe_unused]] const std::filesystem::path &sceneFolder) {
  auto result = Environment{};

  result.name = strAttribOr(element, "name", "");
  result.skyboxRotation = xmlStrToGlmVec3(strAttribOr(element, "skyboxrot", "0 0 0"));
  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  result.raining = element->BoolAttribute("rain", false);
  result.skyboxTint = xmlStrToGlmVec3(strAttribOr(element, "skyboxtint", "0 0 0"));
  result.skyboxBrightness = element->FloatAttribute("brightness", 1);
  result.ambient = element->FloatAttribute("ambient", .1);
  result.brightness = element->FloatAttribute("brightness", 1);
  return result;
}
Water Water::FromXml(tinyxml2::XMLElement *element, [[maybe_unused]] const std::filesystem::path &sceneFolder) {
  auto result = Water{};

  result.position = xmlStrToGlmVec3(strAttribOr(element, "pos", "0 0 0"));
  result.depth = element->FloatAttribute("depth", .1);
  return result;
}
void VoxDataGroup::loadRawVoxelData(
    std::unordered_map<std::string, std::unique_ptr<pf::vox::RawVoxelScene>> &fileCache) {
  std::ranges::for_each(voxData, [&fileCache](VoxData &data) { data.loadRawVoxelData(fileCache); });
  std::ranges::for_each(groups, [&fileCache](VoxDataGroup &data) { data.loadRawVoxelData(fileCache); });
}
void VoxData::loadRawVoxelData(std::unordered_map<std::string, std::unique_ptr<pf::vox::RawVoxelScene>> &fileCache) {
  if (auto iter = fileCache.find(file.string()); iter != fileCache.end()) {
    pf::logd(pf::MAIN_TAG, "Using cached object: {} from file: {}", objectName, file.string());
    if (objectName.empty()) {
      rawVoxelData = std::make_unique<pf::vox::RawVoxelScene>(*iter->second);
    } else {
      rawVoxelData = std::make_unique<pf::vox::RawVoxelModel>(iter->second->getModelByName(objectName));
    }
  } else {
    pf::logd(pf::MAIN_TAG, "Loading file: {}", file.string());
    auto scene = std::make_unique<pf::vox::RawVoxelScene>(pf::vox::loadScene(file));
    if (objectName.empty()) {
      rawVoxelData = std::make_unique<pf::vox::RawVoxelScene>(*scene);
    } else {
      rawVoxelData = std::make_unique<pf::vox::RawVoxelModel>(scene->getModelByName(objectName));
    }
    fileCache[file.string()] = std::move(scene);
  }
}
}// namespace TeardownMap