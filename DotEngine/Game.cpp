#include "Game.h"
#include "Debug.h"
#include "Dot.h"
#include "DotRenderer.h"
#include "QuadTree.h"
#include "Settings.h"
#include "glm/glm.hpp"
#include <chrono>
#include <cstdlib>

Game::Game(DotRenderer *aRenderer)
    : renderer(aRenderer)
{
  for (size_t i = 0; i < Settings::DOTS_AMOUNT; i++) {
    Dot *d = new Dot({std::rand() % Settings::SCREEN_WIDTH, std::rand() % Settings::SCREEN_HEIGHT});
    dots.push_back(d);
  }
  Debug::Log("GAME: Size of dot: " + std::to_string(sizeof(*dots[0])));
  Debug::Log("GAME: Created dots");

  KeySettings settings;
  settings.textColor = {100, 255, 100, 255};
  Debug::UpdateKeySettings("RenderTime", settings);
  Debug::UpdateKeySettings("CollisionTime", settings);
  Debug::UpdateKeySettings("UpdateTime", settings);
  Debug::UpdateKeySettings("QuadTime", settings);

  timeSinceUpdate = 0.0f;
  quadTree = std::make_unique<QuadTree>(
      AABB(0, 0, Settings::SCREEN_WIDTH, Settings::SCREEN_HEIGHT));

  for (Dot *d : dots) {
    quadTree->insert(d);
  }
}

Game::~Game() { CleanUp(); }

void Game::Update(float aDeltaTime) {
  auto start = std::chrono::high_resolution_clock::now();

  // increment quadTreeTime
  timeSinceUpdate += aDeltaTime;
  // reset and create quadTree
  if (timeSinceUpdate >= Settings::QUAD_TREE_REFRESH_RATE / 1000.f) {
    quadTree->rebuild(dots);
  }
    timeSinceUpdate = 0.0f;
  // quadTree->rebuild(dots);
  auto QuadTime_ch = std::chrono::high_resolution_clock::now();

  for (Dot *d : dots) {
    d->Update(aDeltaTime);
  }
  auto UpdateTime_ch = std::chrono::high_resolution_clock::now();

  processCollisions();
  auto CollisionTime_ch = std::chrono::high_resolution_clock::now();

  for (Dot *d : dots) {
    if (d != nullptr) {
      d->Render(renderer);
    }
  }

  // render bounds
  std::vector<AABB> allBounds = quadTree->getAllBounds();
  renderer->SetDrawColor(100, 100, 100, 20);
  for(auto& bound : allBounds){
    renderer->DrawRect(bound.minX, bound.minY, bound.maxX, bound.maxY);
  }

  auto RenderTime_ch = std::chrono::high_resolution_clock::now();

  // DEBUG INFORMATION

  // Quad Time info
  int QuadTime_millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(QuadTime_ch - start)
          .count();
  std::string QuadTime_str =
      "QuadTime: " + std::to_string(QuadTime_millis) + "ms";
  Debug::UpdateScreenField("QuadTime", QuadTime_str);
  // Update time info
  int UpdateTime_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                              UpdateTime_ch - QuadTime_ch)
                              .count();
  std::string UpdateTime_str =
      "UpdateTime: " + std::to_string(UpdateTime_millis) + "ms";
  Debug::UpdateScreenField("UpdateTime", UpdateTime_str);
  // Collision time info
  int CollisionTime_millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(CollisionTime_ch -
                                                            UpdateTime_ch)
          .count();
  std::string CollisionTime_str =
      "CollisionTime: " + std::to_string(CollisionTime_millis) + "ms";
  Debug::UpdateScreenField("CollisionTime", CollisionTime_str);
  // Render time info
  int RenderTime_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                              RenderTime_ch - CollisionTime_ch)
                              .count();
  std::string RenderTime_str =
      "RenderTime: " + std::to_string(RenderTime_millis) + "ms";
  Debug::UpdateScreenField("RenderTime", RenderTime_str);
}

void Game::processCollisions() {
  for (Dot *d1 : dots) {
    if (d1 == nullptr)
      continue;

    if (d1->radius >= Settings::DOT_RADIUS + 3) {
      d1->Init({std::rand() % Settings::SCREEN_WIDTH, std::rand() % Settings::SCREEN_HEIGHT});
      // quadTree->insert(d1);
      continue;
    }

    float radius = d1->radius * 1.5f;
    AABB queryBounds{d1->position.x - radius, d1->position.y - radius,
                     d1->position.x + radius, d1->position.y + radius};
    quadTree->query(queryBounds, [&](Dot *d2) {
      if (d1 != d2 && d2 > d1 && d2->radius < Settings::DOT_RADIUS + 3) {
        collideDots(d1, d2);
      }
    });
    // for(Dot* d2 : dots){
    //   if (d1 != d2 && d2 > d1) {
    //     collideDots(d1, d2);
    //   }
    // }
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
  quadTree = std::make_unique<QuadTree>(
      AABB(0, 0, Settings::SCREEN_WIDTH, Settings::SCREEN_HEIGHT));

  for (Dot *d : dots) {
    quadTree->insert(d);
  }
}

void Game::CleanUp() {
  // delete dots
  for (size_t i = 0; i < dots.size(); ++i) {
    delete dots[i];
    dots[i] = nullptr;
  }
}
