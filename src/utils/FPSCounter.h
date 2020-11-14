//
// Created by petr on 11/9/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UTILS_FPSCOUNTER_H
#define REALISTIC_VOXEL_RENDERING_SRC_UTILS_FPSCOUNTER_H

#include <chrono>
#include <functional>

namespace pf {

class FPSCounter {
 public:
  FPSCounter() = default;

  void onFrame();
  void reset();

  void setOnNewFrame(std::invocable<const FPSCounter&> auto callback) {
    onNewFrame = callback;
  }

  [[nodiscard]] float averageFPS() const;
  [[nodiscard]] float currentFPS() const;

  [[nodiscard]] std::chrono::milliseconds currentDuration() const;
  [[nodiscard]] std::chrono::milliseconds averageDuration() const;
 private:
  std::size_t totalFrameCount = 0;
  std::chrono::milliseconds frameDuration{};
  std::chrono::milliseconds totalTime{};
  std::chrono::steady_clock::time_point lastFrame = std::chrono::steady_clock::now();

  std::function<void(const FPSCounter &)> onNewFrame = [] (auto) {};
};

}
#endif//REALISTIC_VOXEL_RENDERING_SRC_UTILS_FPSCOUNTER_H
