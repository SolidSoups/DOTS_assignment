#pragma once
#include "glm/glm.hpp"
#include <cstdint>
#include <cstdio>
#include <random>

class DotRenderer;

class Dots {
public:
  static constexpr int MAX_DOTS = 9000; // 9000 * 17B = 153kB
  static constexpr float VELOCITY = 50.f;
  static constexpr int RADIUS = 1;

private: // randomness
  thread_local static std::mt19937 rng;
  thread_local static std::uniform_real_distribution<float> angleDist;
  thread_local static std::uniform_int_distribution<int> xDist;
  thread_local static std::uniform_int_distribution<int> yDist;
  thread_local static bool initialized;

  void ensureRngInit();

public:
  Dots();
  ~Dots();
  void init();

  void initDot(size_t index);

  void updateAll(float deltaTime);
  void renderAll(DotRenderer *aRenderer);

  inline const size_t size() const { return MAX_DOTS; }

public:
  float positions_x[MAX_DOTS];  // 4B
  float positions_y[MAX_DOTS];  // 4B
  float velocities_x[MAX_DOTS]; // 4B
  float velocities_y[MAX_DOTS]; // 4B
  uint8_t radii[MAX_DOTS];      // 1B per
};
