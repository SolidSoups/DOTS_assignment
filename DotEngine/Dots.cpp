#include "Dots.h"
#include "Debug.h"
#include "DotRenderer.h"
#include "Settings.h"
#include "glm/gtc/constants.hpp"
#include <cstring>
#include <ctime>
#include <random>

thread_local std::mt19937 Dots::rng;
thread_local std::uniform_real_distribution<float> Dots::angleDist;
thread_local std::uniform_int_distribution<int> Dots::xDist;
thread_local std::uniform_int_distribution<int> Dots::yDist;
thread_local bool Dots::initialized = false;

// Constructor
Dots::Dots() {
  std::string dotsCountText = "DOTS_AMOUNT: " + std::to_string(MAX_DOTS);
  Debug::UpdateScreenField("DOTS", dotsCountText);
}

// Deconstructor
Dots::~Dots() {}

void Dots::ensureRngInit() {
  if (!initialized) {
    rng.seed(static_cast<unsigned int>(time(nullptr)));
    angleDist =
        std::uniform_real_distribution<float>(0.0f, 2.0f * glm::pi<float>());
    xDist = std::uniform_int_distribution<int>(0, Settings::SCREEN_WIDTH - 1);
    yDist = std::uniform_int_distribution<int>(0, Settings::SCREEN_HEIGHT - 1);
    initialized = true;
  }
}

// Initializes the whole structure
void Dots::init() {
  ensureRngInit();

  alive_indices.reserve(MAX_DOTS); // avoid capacity thrashing
  for (int i = 0; i < MAX_DOTS; i++) {
    alive_indices.push_back(i);

    // get random x and y positions
    positions_x[i] = xDist(rng);
    positions_y[i] = yDist(rng);

    // get random angle
    float angle = angleDist(rng);

    // calculate said random angle
    velocities_x[i] = std::cos(angle);
    velocities_y[i] = std::sin(angle);

    // give starting radius
    radii[i] = RADIUS;
  }
}

// Reinitializes a single dot
void Dots::initDot(size_t index) {
  ensureRngInit();

  // same randomness
  positions_x[index] = xDist(rng);
  positions_y[index] = yDist(rng);

  float angle = angleDist(rng);
  velocities_x[index] = std::cos(angle);
  velocities_y[index] = std::sin(angle);

  radii[index] = RADIUS;
}

void Dots::updateAll(float deltaTime) {
  for (size_t i : alive_indices) {
    positions_x[i] += velocities_x[i] * VELOCITY * deltaTime;
    positions_y[i] += velocities_y[i] * VELOCITY * deltaTime;

    // X-Axis bounds check
    if (positions_x[i] - radii[i] < 0.0f) {
      positions_x[i] = radii[i];
      velocities_x[i] *= -1.f;
    } else if (positions_x[i] + radii[i] > Settings::SCREEN_WIDTH) {
      positions_x[i] = Settings::SCREEN_WIDTH - radii[i];
      velocities_x[i] *= -1.f;
    }

    // Y-Axis bounds check
    if (positions_y[i] - radii[i] < 0.0f) {
      positions_y[i] = radii[i];
      velocities_y[i] *= -1.f;
    } else if (positions_y[i] + radii[i] > Settings::SCREEN_HEIGHT) {
      positions_y[i] = Settings::SCREEN_HEIGHT - radii[i];
      velocities_y[i] *= -1.f;
    }
  }
}

// Renders all the dots
void Dots::renderAll(DotRenderer *aRenderer, Timer& timer) {
  aRenderer->BatchDrawCirclesCPUThreaded(
    positions_x,
    positions_y,
    radii, 
    alive_indices,
    timer);
}
