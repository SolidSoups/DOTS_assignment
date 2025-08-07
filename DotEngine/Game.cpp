#include "Game.h"
#include "Debug.h"
#include "Dot.h"
#include "DotRenderer.h"
#include "QuadTree.h"
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
  Debug::Log("GAME: Size of dot: " + std::to_string(sizeof(*dots[0])));
  Debug::Log("GAME: Created dots");

  timeSinceUpdate = 0.0f;
  createQuadTree();
}

Game::~Game() { CleanUp(); }

void Game::Update(float aDeltaTime) {
  // increment quadTreeTime
  timeSinceUpdate += aDeltaTime;
  // reset and create quadTree
  if (timeSinceUpdate >= QUAD_TIME_MILLIS / 1000.f) {
    createQuadTree(); // don't create, update!
    timeSinceUpdate = 0.0f;
  }

  processCollisions();
  for (Dot *d : dots) {
    if (d != nullptr) {
      d->Render(renderer, aDeltaTime);
    }
  }
}

void Game::processCollisions() {
  for (Dot *d1 : dots) {
    if (d1 == nullptr)
      continue;

    std::vector<Dot *> dotQuery;
    quadTree->query(AABB(d1->position.x - RANGE_CHECK_SIZE / 2.f,
                         d1->position.y - RANGE_CHECK_SIZE / 2.f,
                         d1->radius + RANGE_CHECK_SIZE,
                         d1->radius + RANGE_CHECK_SIZE),
                    dotQuery);

    for (Dot *d2 : dotQuery) {
      if (d1 != d2 && d1 != nullptr && d2 != nullptr) {
        float dist = glm::distance(d2->position, d1->position);
        float minDist = d1->radius + d2->radius;

        if (dist < minDist) {
          glm::vec2 normal = glm::normalize(d2->position - d1->position);

          d1->velocity = glm::reflect(d1->velocity, normal);
          d2->velocity = glm::reflect(d2->velocity, -normal);

          float overlap = (minDist - dist);
          d1->position += normal * overlap;
          d2->position -= normal * overlap;
          d1->radius++;
          d2->radius++;
        }
      }
    }
    if (d1->radius >= 6) {
      d1->Init({std::rand() % SCREEN_WIDTH, std::rand() % SCREEN_HEIGHT}, 3);
      quadTree->insert(d1);
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
}

void Game::CleanUp() {
  // delete tree
  if (quadTree) {
    delete quadTree;
    quadTree = nullptr;
  }

  // delete dots
  for (int i = 0; i < dots.size(); ++i) {
    delete dots[i];
    dots[i] = nullptr;
  }
}
