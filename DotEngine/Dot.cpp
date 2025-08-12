#include "Dot.h"
#include "DotRenderer.h"
#include "Game.h"
#include <ctime>
#include <glm/gtc/constants.hpp>
#include <random>
#include "DotSettings.h"

Dot::Dot(glm::vec2 aPosition) {
  Init(aPosition);
}

void Dot::Init(glm::vec2 aPosition){
  position = aPosition;
  startPos = aPosition;
  radius = globalSettings.DOT_RADIUS;

  static std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
  std::uniform_real_distribution<float> dist(-100.0f, 100.0f);

  velocity = glm::vec2(dist(rng), dist(rng));

  float angle = dist(rng) * glm::pi<float>() / 100.0f;
  velocity = glm::vec2(cos(angle), sin(angle));
}

void Dot::Update(float dt){
  totalTime += dt;
  position += velocity * globalSettings.DOT_VELOCITY * dt;
}

void Dot::Render(DotRenderer *aRenderer, float dt) {
  const float foo = 0.5f * 255.0f;

  // instead show how much life is left
  float redColor = foo + (radius - globalSettings.DOT_RADIUS) * foo / 3.f;
  // float greenColor = glm::cos((totalTime + startPos.y) * 0.9f) * foo + foo;
  // float blueColor = glm::cos(totalTime * 0.4f) * foo + foo;
  // float redColor = 255;        // no significant performance increase
  float greenColor = foo;
  float blueColor = foo;

  aRenderer->SetDrawColor(redColor, greenColor, blueColor, 255);

  aRenderer->DrawFilledCircle(position.x, position.y, radius);

  // haven't checked this shit, but maybe not during a render call?
  if (position.x - radius < 0.0f) {
    position.x = radius;
    velocity.x *= -1;
  } else if (position.x + radius > globalSettings.SCREEN_WIDTH) {
    position.x = globalSettings.SCREEN_HEIGHT - radius;
    velocity.x *= -1;
  }

  if (position.y - radius < 0.0f) {
    position.y = radius;
    velocity.y *= -1;
  } else if (position.y + radius > globalSettings.SCREEN_HEIGHT) {
    position.y = globalSettings.SCREEN_HEIGHT - radius;
    velocity.y *= -1;
  }
}
