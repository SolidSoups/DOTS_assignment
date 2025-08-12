#pragma once
#include "glm/glm.hpp"
#include <cstdint>

class DotRenderer;

class Dot // 32 bytes !!!!
{
public:

	Dot(glm::vec2 aPosition);
  void Init(glm::vec2 aPosition);
  void Update(float dt);
  void Render(DotRenderer* aRenderer, float dt);

	glm::vec2 position;   // 8B
	glm::vec2 startPos;   // 8B
	glm::vec2 velocity;   // 8B

	float totalTime = 0;  // 4B

  uint16_t radius;     // 2B 0-255
};

