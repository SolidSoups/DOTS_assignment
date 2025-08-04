#include "Dot.h"
#include "DotRenderer.h"
#include "Game.h"
#include <random>
#include <glm/gtc/constants.hpp>

const float DotVelocity = 50.0f;

Dot::Dot(glm::vec2 aPosition, float aRadius)
{
	position = aPosition;
	startPos = aPosition;
	Radius = aRadius;

	static std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));
	std::uniform_real_distribution<float> dist(-100.0f, 100.0f);

	velocity = glm::vec2(dist(rng), dist(rng));

	float angle = dist(rng) * glm::pi<float>() / 100.0f;
	velocity = glm::vec2(cos(angle), sin(angle));

	health = 3;
}

void Dot::Render(DotRenderer* aRenderer, float dt)
{
	totalTime += dt;

	position += velocity * DotVelocity * dt;

	if (!overriden)
	{

		float redColor = (glm::cos((totalTime + startPos.x) * 0.1f) * 0.5f + 0.5f) * 255.0f;

		float greenColor = (glm::cos((totalTime + startPos.y) * 0.9f) * 0.5f + 0.5f) * 255.0f;

		float blueColor = (glm::cos(totalTime * 0.4f) * 0.5f + 0.5f) * 255.0f;

		aRenderer->SetDrawColor(redColor, greenColor, blueColor, 255);
	}
	else 
	{
		aRenderer->SetDrawColor(255, 255, 255, 255);
	}

	aRenderer->DrawFilledCircle(position.x, position.y, Radius);

	if (position.x - Radius < 0.0f)
	{
		position.x = Radius;
		velocity.x *= -1;
	}
	else if (position.x + Radius > SCREEN_WIDTH)
	{
		position.x = SCREEN_WIDTH - Radius;
		velocity.x *= -1;
	}

	if (position.y - Radius < 0.0f)
	{
		position.y = Radius;
		velocity.y *= -1;
	}
	else if (position.y + Radius > SCREEN_HEIGHT)
	{
		position.y = SCREEN_HEIGHT - Radius;
		velocity.y *= -1;
	}
}

void Dot::TakeDamage(int someDamage)
{
	health -= someDamage;
}



