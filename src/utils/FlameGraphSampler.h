//
// Created by petr on 11/9/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UTILS_FLAMEGRAPHSAMPLER_H
#define REALISTIC_VOXEL_RENDERING_SRC_UTILS_FLAMEGRAPHSAMPLER_H

#include <pf_imgui/elements/FlameGraph.h>

namespace pf {

namespace details {

struct FlameGraphSamplerBase {
  ~FlameGraphSamplerBase() = default;
  virtual void saveSample(ui::ig::FlameGraphSample &&sample) = 0;
  [[nodiscard]] virtual uint8_t getLevel() const = 0;
};

}// namespace details

class BlockFlameGraphSampler : public details::FlameGraphSamplerBase {
 public:
  BlockFlameGraphSampler(FlameGraphSamplerBase &owner, std::string sampleCaption,
                         std::chrono::steady_clock::time_point startPoint, uint8_t sampleLevel = 0);

  BlockFlameGraphSampler(const BlockFlameGraphSampler &) = delete;
  BlockFlameGraphSampler &operator=(const BlockFlameGraphSampler &) = delete;

  void end();
  void saveSample(ui::ig::FlameGraphSample &&sample) override;
  [[nodiscard]] uint8_t getLevel() const override;

  BlockFlameGraphSampler blockSampler(std::string subCaption);

  ~BlockFlameGraphSampler();

 private:
  void saveToParent();

  FlameGraphSamplerBase &parent;
  std::string caption;
  pf::math::Range<std::chrono::microseconds> time{};
  std::chrono::steady_clock::time_point firstTimePoint;

  uint8_t level;
  bool isSaved = false;

  std::vector<ui::ig::FlameGraphSample> childSamples;
};

class FlameGraphSampler : public details::FlameGraphSamplerBase {
 public:
  BlockFlameGraphSampler blockSampler(std::string subCaption);

  void saveSample(ui::ig::FlameGraphSample &&sample) override;
  [[nodiscard]] uint8_t getLevel() const override;

  [[nodiscard]] const std::vector<ui::ig::FlameGraphSample> &getSamples() const;

 private:
  std::vector<ui::ig::FlameGraphSample> samples;
};

}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_UTILS_FLAMEGRAPHSAMPLER_H
