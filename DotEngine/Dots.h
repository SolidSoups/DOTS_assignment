#pragma once
#include "glm/glm.hpp"
#include <cstdint>
#include <cstdio>
#include <random>
#include "SimpleProfiler.h"

class DotRenderer;

class Dots {
public:
  static constexpr int MAX_DOTS = 25000; // 20000 * 17B = 340kB
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

  /*
   * Re-Initializes a dot with new position and velocity
   *
   * @param index The index of the dot
   */
  void initDot(size_t index);

  /*
  * Moves all dots by their velocities and updates bounces on borders
  *
  * @param deltaTime deltaTime
  */
  void updateAll(float deltaTime);
  /*
  * Renders every dot on the screen
  *
  * @param aRenderer Pointer to the DotRenderer instance
  * @param timer Reference to a benchmark timer
  */
  void renderAll(DotRenderer *aRenderer, Timer& timer);

  inline const size_t size() const { return MAX_DOTS; }

public:
  float positions_x[MAX_DOTS];  // 4B
  float positions_y[MAX_DOTS];  // 4B
  float velocities_x[MAX_DOTS]; // 4B
  float velocities_y[MAX_DOTS]; // 4B
  uint8_t radii[MAX_DOTS];      // 1B per

public:
  // keeping track of dead and alive indices means we can
  // entirely skip over instructions at every part of the
  // pipeline increasing performance (culling)
  std::vector<size_t> alive_indices;
  std::vector<size_t> dead_indices;
};
