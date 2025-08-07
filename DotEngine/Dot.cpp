#include "Dot.h"
#include "DotRenderer.h"
#include "Game.h"
#include <ctime>
#include <glm/gtc/constants.hpp>
#include <random>

const float DotVelocity = 50.0f;

Dot::Dot(glm::vec2 aPosition, float aRadius) {
  Init(aPosition, aRadius);
}

void Dot::Init(glm::vec2 aPosition, float aRadius){
  position = aPosition;
  startPos = aPosition;
  radius = aRadius;

  static std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
  std::uniform_real_distribution<float> dist(-100.0f, 100.0f);

  velocity = glm::vec2(dist(rng), dist(rng));

  float angle = dist(rng) * glm::pi<float>() / 100.0f;
  velocity = glm::vec2(cos(angle), sin(angle));
}

void Dot::Render(DotRenderer *aRenderer, float dt) {
  const float foo = 0.5f * 255.0f;
  totalTime += dt;

  position += velocity * DotVelocity * dt;

  // removed 6 * operations
  float redColor = glm::cos((totalTime + startPos.x) * 0.1f) * foo + foo;
  float greenColor = glm::cos((totalTime + startPos.y) * 0.9f) * foo + foo;
  float blueColor = glm::cos(totalTime * 0.4f) * foo + foo;
  // float redColor = 255;        // no significant performance increase
  // float greenColor = 255;
  // float blueColor = 255;

  aRenderer->SetDrawColor(redColor, greenColor, blueColor, 255);

  aRenderer->DrawFilledCircle(position.x, position.y, radius);

  if (position.x - radius < 0.0f) {
    position.x = radius;
    velocity.x *= -1;
  } else if (position.x + radius > SCREEN_WIDTH) {
    position.x = SCREEN_WIDTH - radius;
    velocity.x *= -1;
  }

  if (position.y - radius < 0.0f) {
    position.y = radius;
    velocity.y *= -1;
  } else if (position.y + radius > SCREEN_HEIGHT) {
    position.y = SCREEN_HEIGHT - radius;
    velocity.y *= -1;
  }
}
