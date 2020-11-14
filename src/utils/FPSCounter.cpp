//
// Created by petr on 11/9/20.
//

#include "FPSCounter.h"

namespace pf {
using namespace std::chrono_literals;

float FPSCounter::averageFPS() const {
  return static_cast<float>(totalFrameCount) / totalTime.count()
      * std::chrono::duration_cast<std::chrono::milliseconds>(1s).count();
}

float FPSCounter::currentFPS() const {
  return 1.f / frameDuration.count()
      * std::chrono::duration_cast<std::chrono::milliseconds>(1s).count();
}
std::chrono::milliseconds FPSCounter::currentDuration() const { return frameDuration; }
std::chrono::milliseconds FPSCounter::averageDuration() const {
  return totalTime / totalFrameCount;
}
void FPSCounter::onFrame() {
  ++totalFrameCount;
  const auto now = std::chrono::steady_clock::now();
  frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrame);
  totalTime += frameDuration;
  lastFrame = now;
  onNewFrame(*this);
}
void FPSCounter::reset() {
  totalFrameCount = 0;
  frameDuration = 0ms;
  totalTime = 0ms;
  lastFrame = std::chrono::steady_clock::now();
}
}// namespace pf