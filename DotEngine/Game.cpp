#include "Game.h"
#include "DotRenderer.h"
#include "Dot.h"
#include <cstdlib>
#include "glm/glm.hpp"
#include <algorithm>


std::vector<Dot*> dots;

const int DotAmount = 500;

Game::Game(DotRenderer* aRenderer)
{
	renderer = aRenderer;

	for (size_t i = 0; i < DotAmount; i++)
	{
		int diry = std::rand() % 2;
		int dirx = std::rand() % 2;

		dirx = -1 ? dirx > 1 : dirx;
		diry = -1 ? diry > 1 : diry;

		Dot* d = new Dot({ std::rand() % SCREEN_WIDTH, std::rand() % SCREEN_HEIGHT }, 3);

		dots.push_back(d);
	}

	//To debug collision
	dots[0]->overriden = true;
	dots[0]->Radius = 10;
	//To debug collision
}

void Game::Update(float aDeltaTime)
{
	std::vector<Dot*> toDestroy;
	for (Dot* d1 : dots)
	{
		for (Dot* d2 : dots)
		{
			if (d1 != d2 && d1 != nullptr && d2 != nullptr)
			{
				float dist = glm::distance(d1->position, d2->position);
				float minDist = d1->Radius + d2->Radius;

				if (dist < minDist)
				{
					glm::vec2 normal = glm::normalize(d2->position - d1->position);

					d1->velocity = glm::reflect(d1->velocity, normal);
					d2->velocity = glm::reflect(d2->velocity, -normal);

					float overlap1 = 1.5f * ((minDist + 1) - dist);
					float overlap2 = 1.5f * (minDist - dist);
					d1->position -= normal * overlap1;
					d2->position += normal * overlap2;
					d1->TakeDamage(1);
					d1->Radius++;
				}
				if (d1->health <= 0)
				{
					toDestroy.push_back(d1);
				}
			}
		}
	}

	std::vector<int> indexesToRemove;

	for (Dot* d : toDestroy)
	{
		for (size_t i = 0; i < dots.size(); i++)
		{
			if (std::find(indexesToRemove.begin(), indexesToRemove.end(), i) != indexesToRemove.end())
			{
				continue;
			}
			else if (dots[i] == d)
			{
				indexesToRemove.push_back(i);
			}
		}
	}

	for (int i : indexesToRemove)
	{
		dots[i] = nullptr;
		Dot* newDot = new Dot({ std::rand() % SCREEN_WIDTH, std::rand() % SCREEN_HEIGHT }, 3);
		dots.push_back(newDot);
	}

	for (Dot* d : dots)
	{
		if (d != nullptr)
		{
			d->Render(renderer, aDeltaTime);
		}
	}
}

void Game::CleanUp()
{

}
