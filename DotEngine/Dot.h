#pragma once
#include "glm/glm.hpp"

class DotRenderer;

class Dot // 40 bytes !!!!
{
public:

	Dot(glm::vec2 aPosition, float aRadius);
	void Render(DotRenderer* aRenderer, float dt);
	void TakeDamage(int someDamage);
  void hello();

	glm::vec2 position;
	glm::vec2 startPos;
	glm::vec2 velocity;

	float totalTime = 0;
	float Radius = 0;

	int health;

	bool overriden = false;
};

