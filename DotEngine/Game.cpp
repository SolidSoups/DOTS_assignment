#include "Game.h"
#include "Debug.h"
#include "Dot.h"
#include "DotRenderer.h"
#include "DotSettings.h"
#include "QuadTree.h"
#include "glm/glm.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>

Game::Game(DotRenderer *aRenderer)
    : quadTree(nullptr), renderer(aRenderer),
      dot_amount(globalSettings.DOTS_AMOUNT),
      screen_width(globalSettings.SCREEN_WIDTH),
      screen_height(globalSettings.SCREEN_HEIGHT),
      quad_refresh_rate_millis(globalSettings.QUAD_TREE_MILLIS_REFRESH_TIME),
      default_bounds(0.0f, 0.0f, screen_width, screen_height) 
{
  for (size_t i = 0; i < dot_amount; i++) {
    int diry = std::rand() % 2;
    int dirx = std::rand() % 2;

    dirx = -1 ? dirx > 1 : dirx;
    diry = -1 ? diry > 1 : diry;

    Dot *d =
        new Dot({std::rand() % screen_width, std::rand() % screen_height});

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
  if (timeSinceUpdate >= quad_refresh_rate_millis / 1000.f) {
    createQuadTree(); // don't create, update!
    timeSinceUpdate = 0.0f;
  }

  for (Dot *d : dots) {
    d->Update(aDeltaTime);
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

    float queryRadius = d1->radius * 2.f;
    AABB queryBounds{d1->position.x - queryRadius, d1->position.y - queryRadius,
                     queryRadius * 2.f, queryRadius * 2.f};

    std::vector<Dot *> nearbyDots;
    quadTree->query(queryBounds, nearbyDots);

    for (Dot *d2 : nearbyDots) {
      if (d1 != d2 && d2 != nullptr) {
        collideDots(d1, d2);
      }
    }
    if (d1->radius >= globalSettings.DOT_RADIUS + 3) {
      d1->Init({std::rand() % screen_width, std::rand() % screen_height});
      quadTree->insert(d1);
    }
  }
}

void Game::collideDots(Dot *d1, Dot *d2) {
  glm::vec2 diff = d2->position - d1->position;
  float distSq = diff.x * diff.x + diff.y * diff.y;
  float minDist = d1->radius + d2->radius;
  float minDistSq = minDist * minDist;

  if (distSq < minDistSq && distSq > 0.01f) { // division
    float dist = sqrt(distSq);                // only sqrt when colliding
    glm::vec2 normal = diff / dist;           // normallize manuall

    // collision responses
    d1->velocity = glm::reflect(d1->velocity, normal);
    d2->velocity = glm::reflect(d2->velocity, -normal);

    // Seperate dots
    float overlap = (minDist + 2 - dist) * 1.5f;
    d1->position -= normal * overlap;
    d2->position += normal * overlap;

    d1->radius++;
    d2->radius++;
    Debug::Log("[GAME] Collision!");
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
