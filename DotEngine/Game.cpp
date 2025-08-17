#include "Game.h"
#include "Debug.h"
#include "DotRenderer.h"
#include "QuadTree.h"
#include "Settings.h"
#include "glm/glm.hpp"
#include <chrono>
#include <cstdlib>

Game::Game(DotRenderer *aRenderer) : renderer(aRenderer) {
  dots.init();
  Debug::Log("GAME: Size of dot: " + std::to_string(dots.size()));
  Debug::Log("GAME: Created dots");

  // Settings for debug values on screen
  KeySettings settings;
  settings.textColor = {100, 255, 100, 255};
  Debug::UpdateKeySettings("RenderTime", settings);
  Debug::UpdateKeySettings("CollisionTime", settings);
  Debug::UpdateKeySettings("UpdateTime", settings);
  Debug::UpdateKeySettings("QuadTime", settings);

  timeSinceUpdate = 0.0f;
  quadTree = std::make_unique<QuadTree>(
      AABB(0, 0, Settings::SCREEN_WIDTH, Settings::SCREEN_HEIGHT));

  // TODO: dots
  quadTree->rebuild(dots);
}

Game::~Game() {}

void Game::Update(float aDeltaTime) {
  auto start = std::chrono::high_resolution_clock::now();

  // increment rebuild timer
  timeSinceUpdate += aDeltaTime;
  if (timeSinceUpdate >= Settings::QUAD_TREE_REFRESH_RATE / 1000.f) {
    // rebuild and reset timer
    quadTree->rebuild(dots);
    timeSinceUpdate = 0.0f;
  }
  auto QuadTime_ch = std::chrono::high_resolution_clock::now();

  // Update all the dots positions
  dots.updateAll(aDeltaTime);
  auto UpdateTime_ch = std::chrono::high_resolution_clock::now();

  // Process all collisions
  processCollisions();
  auto CollisionTime_ch = std::chrono::high_resolution_clock::now();

  // Render all the dots
  dots.renderAll(renderer);

  // Render all QuadTree bounds
  // std::vector<AABB> allBounds = quadTree->getAllBounds();
  // renderer->SetDrawColor(100, 100, 100, 20);
  // for (auto &bound : allBounds) {
  //   renderer->DrawRect(bound.minX, bound.minY, bound.maxX, bound.maxY);
  // }
  auto RenderTime_ch = std::chrono::high_resolution_clock::now();

  // ####################
  // ## DEBUG TIMINGS: ##
  // ####################

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

SimpleProfiler profiler;
void Game::processCollisions() {
  static std::vector<size_t> alive_indices;
  alive_indices.clear();
  alive_indices.reserve(dots.size());

  for(size_t i=0; i<dots.size(); i++){
    if(dots.radii[i] >= dots.RADIUS + 3){
      dots.initDot(i);
    } else{
      alive_indices.push_back(i);
    }
  }
  for (size_t i1 : alive_indices) {
    // create query bounds
    float radius = dots.radii[i1] * 1.5f;
    float pos_x = dots.positions_x[i1];
    float pos_y = dots.positions_y[i1];
    AABB queryBounds{
      pos_x - radius, 
      pos_y - radius, 
      pos_x + radius,
      pos_y + radius
    };

    // perform query and callback to collision func
    quadTree->query(queryBounds, [&](size_t i2) {
      if (i1 != i2 && i2 > i1 && dots.radii[i2] < Settings::DOT_RADIUS + 3) {
        collideDots(i1, i2);
      }
    });
  }

  static int stat_frame = 0;
  if(++stat_frame % 120 == 0){
    size_t visits = 0, rejected = 0, checks = 0;
    quadTree->getRoot()->collectStats(visits, rejected, checks);
    printf("\nQuadTree stats: \nvisits=%zu rejects=%zu checks=%zu efficiency=%.1f%%\n",
           visits, rejected, checks,
           100.0 * rejected / visits);
    quadTree->getRoot()->resetStats();
  }
}

void Game::collideDots(size_t i1, size_t i2) {
  float p1_x = dots.positions_x[i1];
  float p1_y = dots.positions_y[i1];
  float p2_x = dots.positions_x[i2];
  float p2_y = dots.positions_y[i2];

  float diff_x = p2_x - p1_x;
  float diff_y = p2_y - p1_y;
  float distSq = diff_x * diff_x + diff_y * diff_y;

  uint8_t r1 = dots.radii[i1];
  uint8_t r2 = dots.radii[i2];
  float minDist = r1 + r2;
  float minDistSq = minDist * minDist;

  if (distSq >= minDistSq || distSq <= 0.01f) return; // Early exit

  float v1_x = dots.velocities_x[i1];
  float v1_y = dots.velocities_y[i1];
  float v2_x = dots.velocities_x[i2];
  float v2_y = dots.velocities_y[i2];

  float dist = sqrt(distSq);                // only sqrt when colliding
  float normal_x = diff_x / dist;
  float normal_y = diff_y / dist;

  // reflect vectors along normal
  float dotN = normal_x * v1_x + normal_y * v1_y;
  v1_x = 2.0f * normal_x * dotN;
  v1_y = 2.0f * normal_y * dotN;

  float dotDN = -normal_x * v2_x - normal_y * v2_y;
  v2_x = 2.0f * -normal_x * dotDN;
  v2_y = 2.0f * -normal_y * dotDN;

  // normalize velocities
  float imag1 = 1.f / sqrt(v1_x * v1_x + v1_y * v1_y);
  dots.velocities_x[i1] = v1_x * imag1;
  dots.velocities_y[i1] = v1_y * imag1;

  float imag2 = 1.f / sqrt(v2_x * v2_x + v2_y * v2_y);
  dots.velocities_x[i2] = v2_x * imag2;
  dots.velocities_y[i2] = v2_y * imag2;

  // Seperate dots
  float overlap = (minDist + 2 - dist) * 1.5f;
  dots.positions_x[i1] = p1_x - normal_x * overlap;
  dots.positions_y[i1] = p1_y - normal_y * overlap;
  dots.positions_x[i2] = p2_x + normal_x * overlap;
  dots.positions_y[i2] = p2_y + normal_y * overlap;

  dots.radii[i1] = r1 + 1;
  dots.radii[i2] = r2 + 1;
  // Debug::Log("[GAME] Collision!");
}
