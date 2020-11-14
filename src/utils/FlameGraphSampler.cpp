//
// Created by petr on 11/9/20.
//

#include "FlameGraphSampler.h"

namespace pf {

BlockFlameGraphSampler::BlockFlameGraphSampler(FlameGraphSamplerBase &owner,
                                               std::string sampleCaption,
                                               std::chrono::steady_clock::time_point startPoint,
                                               uint8_t sampleLevel)
    : parent(owner), caption(std::move(sampleCaption)), firstTimePoint(startPoint),
      level(sampleLevel) {
  time.start = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now() - firstTimePoint);
}

BlockFlameGraphSampler BlockFlameGraphSampler::blockSampler(std::string subCaption) {
  return BlockFlameGraphSampler(*this, std::move(subCaption), firstTimePoint, level + 1);
}
BlockFlameGraphSampler::~BlockFlameGraphSampler() {
  if (isSaved) { return; }
  saveToParent();
}
void BlockFlameGraphSampler::saveToParent() {
  auto sample = ui::ig::FlameGraphSample(time, std::move(caption), level);
  std::ranges::for_each(childSamples, [&](const ui::ig::FlameGraphSample &subSample) {
    sample.addSubSample(subSample);
  });
  parent.saveSample(std::move(sample));
  isSaved = true;
}
void BlockFlameGraphSampler::end() {
  const auto now = std::chrono::steady_clock::now();
  time.end = std::chrono::duration_cast<std::chrono::microseconds>(now - firstTimePoint);
  saveToParent();
}

void BlockFlameGraphSampler::saveSample(ui::ig::FlameGraphSample &&sample) {
  childSamples.emplace_back(std::forward<ui::ig::FlameGraphSample>(sample));
}

uint8_t BlockFlameGraphSampler::getLevel() const { return level; }

BlockFlameGraphSampler FlameGraphSampler::blockSampler(std::string subCaption) {
  return BlockFlameGraphSampler(*this, std::move(subCaption), std::chrono::steady_clock::now(), 0);
}

void FlameGraphSampler::saveSample(ui::ig::FlameGraphSample &&sample) {
  samples.emplace_back(std::forward<ui::ig::FlameGraphSample>(sample));
}

uint8_t FlameGraphSampler::getLevel() const { return 0; }

const std::vector<ui::ig::FlameGraphSample> &FlameGraphSampler::getSamples() const {
  return samples;
}
}// namespace pf