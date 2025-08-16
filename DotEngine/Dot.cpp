#include "Dot.h"
#include "DotRenderer.h"
#include "Game.h"
#include <ctime>
#include <glm/gtc/constants.hpp>
#include <random>
#include "Settings.h"

Dot::Dot(glm::vec2 aPosition) {
  Init(aPosition);
}

// creating new instances every frame, huh
void Dot::Init(glm::vec2 aPosition){
  position = aPosition;
  radius = Settings::DOT_RADIUS; // constexpr, no cache miss

  thread_local static std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
  thread_local static std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * glm::pi<float>());

  float angle = angleDist(rng);
  velocity = glm::vec2(std::cos(angle), std::sin(angle));
}

void Dot::Update(float dt){
  position += velocity * Settings::DOT_VELOCITY * dt;

  // haven't checked this shit, but maybe not during a render call?
  if (position.x - radius < 0.0f) {
    position.x = radius;
    velocity.x *= -1;
  } else if (position.x + radius > Settings::SCREEN_WIDTH) {
    position.x = Settings::SCREEN_HEIGHT - radius;
    velocity.x *= -1;
  }

  if (position.y - radius < 0.0f) {
    position.y = radius;
    velocity.y *= -1;
  } else if (position.y + radius > Settings::SCREEN_HEIGHT) {
    position.y = Settings::SCREEN_HEIGHT - radius;
    velocity.y *= -1;
  }
}

void Dot::Render(DotRenderer *aRenderer, float dt) {
  const float foo = 0.5f * 255.0f;

  float redColor = foo + (radius - Settings::DOT_RADIUS) * foo / 3.f;
  float greenColor = foo;
  float blueColor = foo;

  aRenderer->SetDrawColor(redColor, greenColor, blueColor, 255);

  aRenderer->DrawFilledCircle(position.x, position.y, radius);
}
