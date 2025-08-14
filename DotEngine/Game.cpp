#include "Game.h"
#include "Debug.h"
#include "Dot.h"
#include "DotRenderer.h"
#include "DotSettings.h"
#include "QuadTree.h"
#include "glm/glm.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>

Game::Game(DotRenderer *aRenderer)
    : renderer(aRenderer), dot_amount(globalSettings.DOTS_AMOUNT),
      screen_width(globalSettings.SCREEN_WIDTH),
      screen_height(globalSettings.SCREEN_HEIGHT) {
  // create grid
  grid = new FastGrid(globalSettings.SCREEN_WIDTH, globalSettings.SCREEN_HEIGHT,
                      globalSettings.DOT_RADIUS);

  for (size_t i = 0; i < dot_amount; i++) {
    int diry = std::rand() % 2;
    int dirx = std::rand() % 2;

    dirx = -1 ? dirx > 1 : dirx;
    diry = -1 ? diry > 1 : diry;

    Dot *d = new Dot({std::rand() % screen_width, std::rand() % screen_height});

    dots.push_back(d);
    grid->Insert(d);
  }
  Debug::Log("GAME: Size of dot: " + std::to_string(sizeof(*dots[0])));
  Debug::Log("GAME: Created dots");

  KeySettings newSettings;
  newSettings.textColor = {120, 120, 0, 255};
  Debug::UpdateKeySettings("UpdateTime", newSettings);
  Debug::UpdateKeySettings("RenderTime", newSettings);
  Debug::UpdateKeySettings("CollisionTime", newSettings);
}

Game::~Game() { CleanUp(); }

void Game::Update(float aDeltaTime) {
  auto start = std::chrono::high_resolution_clock::now();
  for (Dot *dot : dots) {
    dot->Update(aDeltaTime);

    size_t currentIndex = grid->getCellIndex(dot->position);
    // index was updated
    if (currentIndex != dot->currentCellIndex) {
      grid->UpdateDot(dot);
    }
  }
  auto after_update = std::chrono::high_resolution_clock::now();
  int after_update_millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(after_update -
                                                            start)
          .count();
  std::string updateTimeStr =
      "UpdateTime: " + std::to_string(after_update_millis);
  Debug::UpdateScreenField("UpdateTime", updateTimeStr);

  processCollisions();
  auto after_collision = std::chrono::high_resolution_clock::now();
  int after_collision_millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(after_collision -
                                                            after_update)
          .count();
  std::string collisionTime_str =
      "CollisionTime: " + std::to_string(after_collision_millis);
  Debug::UpdateScreenField("CollisionTime", collisionTime_str);

  for (Dot *d : dots) {
    if (d != nullptr) {
      d->Render(renderer, aDeltaTime);
    }
  }

  auto after_render = std::chrono::high_resolution_clock::now();
  int after_render_millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(after_render -
                                                            after_collision)
          .count();
  std::string renderTimeStr =
      "RenderTime: " + std::to_string(after_render_millis);
  Debug::UpdateScreenField("RenderTime", renderTimeStr);
}

void Game::processCollisions() {
  collisionPairs.clear();
  collisionPairs.reserve(100);
  collectCollisions();
}

void Game::collectCollisions() {
  for (int i = 0; i < dots.size(); ++i) {
    Dot *d1 = dots[i];
    if (d1 == nullptr)
      return;

    glm::ivec2 pos = grid->getGridPos(d1);
    float radius = d1->radius;
    grid->ForEachInQueryArea(pos, radius, [&](Dot* d2){
      if (d1 != d2 && d2 > d1)
        collisionPairs.push_back({d1, d2});
      return true;
    });
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

void Game::CleanUp() {
  if (grid) {
    delete grid;
    grid = nullptr;
  }

  // delete dots
  for (int i = 0; i < dots.size(); ++i) {
    delete dots[i];
    dots[i] = nullptr;
  }
}
