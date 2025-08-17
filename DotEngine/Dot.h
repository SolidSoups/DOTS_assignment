#pragma once
#include "glm/glm.hpp"
#include <cstdint>
#include <memory>

class DotRenderer;
class QuadTree;

class Dot // 32B
{
public:

	Dot(glm::vec2 aPosition);
  void Init(glm::vec2 aPosition);
  void Update(float dt);
  void Render(DotRenderer* aRenderer);

	glm::vec2 position;   // 8B
	glm::vec2 velocity;   // 8B
  
  // pack at end to avoid misalignment, would be 40B otherwise
  uint16_t radius;     // 2B
};
