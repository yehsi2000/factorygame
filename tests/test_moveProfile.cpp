#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "Components/TransformComponent.h"
#include "Core/EventDispatcher.h"
#include "Core/Registry.h"
#include "DataStruct/Type.h"

// Simple timer class for high-resolution timing
class Timer {
 public:
  void Reset() { start_ = std::chrono::high_resolution_clock::now(); }
  
  double Elapsed() {
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start_).count();
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

int main() {
  std::cout << "Starting Stress Test (Target: 60 FPS, Stop < 30 FPS)..." << std::endl;
  std::cout << "--------------------------------------------------------" << std::endl;
  std::cout << "Entities | Batch Time | Avg FPS | Avg Frame Time (ms)" << std::endl;
  std::cout << "--------------------------------------------------------" << std::endl;

  EventDispatcher eventDispatcher;
  Registry registry(&eventDispatcher);

  int entityCount = 0;
  const int increment = 1000;
  const int framesPerBatch = 60;
  const double targetFrameTime = 1.0 / 60.0; // ~16.67 ms
  float totalSimTime = 0.0f;
  const float deltaTime = 0.016f;

  while (true) {
    // Add entities
    for (int i = 0; i < increment; ++i) {
      auto entity = registry.CreateEntity();
      registry.EmplaceComponent<TransformComponent>(entity, Vec2f{0.f, 0.f});
    }
    entityCount += increment;

    // Measure Batch
    double batchTotalTime = 0.0;

    for (int frame = 0; frame < framesPerBatch; ++frame) {
      auto frameStart = std::chrono::high_resolution_clock::now();
      totalSimTime += deltaTime;

      // 1. Simulate Logic (Update positions)
      auto view = registry.view<TransformComponent>();
      for (auto entity : view) {
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        // Simple movement logic
        transform.position = Vec2f(std::sin(totalSimTime) * 5.0f, std::cos(totalSimTime) * 5.0f);
      }

      auto logicEnd = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> logicDuration = logicEnd - frameStart;
      double logicSeconds = logicDuration.count();

      auto frameEnd = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> frameDuration = frameEnd - frameStart;
      batchTotalTime += frameDuration.count();
    }

    double avgFps = framesPerBatch / batchTotalTime;
    double avgFrameTimeMs = (batchTotalTime / framesPerBatch) * 1000.0;

    std::cout << std::setw(8) << entityCount 
              << " | " << std::fixed << std::setprecision(4) << batchTotalTime << "s"
              << " | " << std::setw(7) << std::setprecision(2) << avgFps 
              << " | " << std::setw(8) << std::setprecision(2) << avgFrameTimeMs << "ms"
              << std::endl;

    if (avgFps < 30.0) {
      std::cout << "--------------------------------------------------------" << std::endl;
      std::cout << "FPS dropped below 30 at " << entityCount << " entities." << std::endl;
      break;
    }
  }

  return 0;
}