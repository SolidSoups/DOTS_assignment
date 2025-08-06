#include "Game.h"
#include "Dot.h"
#include "DotRenderer.h"
#include "QuadTree.h"
#include "Debug.h"
#include "glm/glm.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>

Game::Game(DotRenderer *aRenderer, const int dotAmount)
    : default_bounds(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT),
      quadTree(nullptr), dot_amount(dotAmount) {
  renderer = aRenderer;

  for (size_t i = 0; i < dot_amount; i++) {
    int diry = std::rand() % 2;
    int dirx = std::rand() % 2;

    dirx = -1 ? dirx > 1 : dirx;
    diry = -1 ? diry > 1 : diry;

    Dot *d =
        new Dot({std::rand() % SCREEN_WIDTH, std::rand() % SCREEN_HEIGHT}, 3);

    dots.push_back(d);
  }

  Debug::Log("GAME: Created dots");

  timeSinceUpdate = 0.0f;
  createQuadTree();
}

void Game::Update(float aDeltaTime) {
  // increment quadTreeTime
  timeSinceUpdate += aDeltaTime;
  // reset and create quadTree
  if (timeSinceUpdate >= QUAD_TIME_MILLIS / 1000.f) {
    createQuadTree();
    timeSinceUpdate = 0.0f;
  }

  std::vector<Dot *> toDestroy;
  for (Dot *d1 : dots) {
    if (d1 == nullptr)
      continue;

    std::vector<Dot *> dotQuery;
    quadTree->query(AABB(d1->position.x - RANGE_CHECK_SIZE / 2.f,
                         d1->position.y - RANGE_CHECK_SIZE / 2.f,
                         RANGE_CHECK_SIZE, RANGE_CHECK_SIZE),
                    dotQuery);

    for (Dot *d2 : dotQuery) {
      if (d1 != d2 && d1 != nullptr && d2 != nullptr) {
        float dist = glm::distance(d1->position, d2->position);
        float minDist = d1->Radius + d2->Radius;

        if (dist < minDist) {
          glm::vec2 normal = glm::normalize(d2->position - d1->position);

          d1->velocity = glm::reflect(d1->velocity, normal);
          d2->velocity = glm::reflect(d2->velocity, -normal);

          float overlap1 = 1.5f * ((minDist + 1) - dist);
          float overlap2 = 1.5f * (minDist - dist);
          d1->position -= normal * overlap1;
          d2->position += normal * overlap2;
          d1->TakeDamage(1);
          d2->TakeDamage(1);
          d1->Radius++;
        }
      }
    }
    if (d1->health <= 0) {
      toDestroy.push_back(d1);
    }
  }

  for (Dot *deadDot : toDestroy) {
    auto it = std::find(dots.begin(), dots.end(), deadDot);
    if (it != dots.end()) {
      delete *it;
      *it =
          new Dot({std::rand() % SCREEN_WIDTH, std::rand() % SCREEN_HEIGHT}, 3);
    }
  }

  for (Dot *d : dots) {
    if (d != nullptr) {
      d->Render(renderer, aDeltaTime);
    }
  }
}

void Game::createQuadTree() {
  if (quadTree != nullptr) {
    delete quadTree;
    quadTree = nullptr;
  }

  quadTree = new QuadTree(default_bounds);
  for (Dot *d : dots) {
    quadTree->insert(d);
  }
  std::cout << "GAME: Created a new Quad Tree" << "\n";
}

void Game::CleanUp() {}
