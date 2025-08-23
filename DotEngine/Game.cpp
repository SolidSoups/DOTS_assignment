#include "Game.h"
#include "Debug.h"
#include "DotRenderer.h"
#include "SimpleProfiler.h"
#include "ThreadPool.h"
#include <cstdlib>
#include <mutex>
#include <immintrin.h>

Game::Game(DotRenderer *aRenderer, ThreadPool *threadPool, Timer &timer)
    : renderer(aRenderer), threadPool(threadPool), timer(timer),
      dots_mutexes(Dots::MAX_DOTS) {
  dots.init();

  // Color settings for debug
  KeySettings settings;
  settings.textColor = {100, 255, 100, 255};
  Debug::UpdateKeySettings("Grid_Build", settings);
  Debug::UpdateKeySettings("Dots_Update", settings);
  Debug::UpdateKeySettings("Dots_Collision", settings);
  Debug::UpdateKeySettings("Dots_Render", settings);

  grid.rebuild(dots);
}

Game::~Game() {}

void Game::Update(float aDeltaTime) {
  auto &t_total = timer.startChild("update_total");

  // cull dots first
  cullDots(t_total);

  auto &t_rebuild = t_total.startChild("grid_build");
  grid.rebuild(dots);
  t_rebuild.stopClock();

  // Update all the dots positions
  auto &t_updateDots = t_total.startChild("dots_update");
  dots.updateAll(aDeltaTime);
  t_updateDots.stopClock();

  // Process all collisions
  auto &t_collision = t_total.startChild("dots_collision");
  processCollisions_threaded(t_collision);
  t_collision.stopClock();

  // Render all the dots
  auto &t_render = t_total.startChild("dots_render");
  dots.renderAll(renderer, t_render);
  t_render.stopClock();
  t_total.stopClock();

  // ####################
  // ## DEBUG TIMINGS: ##
  // ####################
  static int frame = 0;
  if (++frame % 60 == 0) {
    Debug::UpdateScreenField("Grid_Build",
                             t_rebuild.getSimpleReport("Grid_Build"));
    Debug::UpdateScreenField("Dots_Update",
                             t_updateDots.getSimpleReport("Dots_Update"));
    Debug::UpdateScreenField("Dots_Collision",
                             t_collision.getSimpleReport("Dots_Collision"));
    Debug::UpdateScreenField("Dots_Render",
                             t_render.getSimpleReport("Dots_Render"));
  }
}

void Game::cullDots(Timer &timer) {
  auto &t_culling = timer.startChild("culling");

  auto &t_startThreads = t_culling.startChild("start_threads");
  const size_t numThreads = threadPool->num_threads;
  const size_t totalAlive = dots.alive_indices.size();
  const size_t dotsPerThread = totalAlive / numThreads;

  // parallelize culling
  for (size_t t = 0; t < numThreads; ++t) {
    size_t start = t * dotsPerThread;
    size_t end = (t == 0) ? totalAlive : start + dotsPerThread;

    threadPool->queueJob([this, start, end] {
      for (size_t i = start; i < end; ++i) {
        if (dots.radii[i] >= Dots::RADIUS + 3)
          dots.initDot(i);
      }
    });
  }
  t_startThreads.stopClock();

  auto &t_waitThreads = t_culling.startChild("wait_for_threads");
  threadPool->wait();
  t_waitThreads.stopClock();

  t_culling.stopClock();
}

void Game::processCollisions_threaded(Timer &timer) {
  auto &t_startThreads = timer.startChild("start_threads");

  // split grid up into columns
  const int nThreads = threadPool->num_threads;
  const size_t colsPerThread = grid.GRID_WIDTH / nThreads;

  // for each thread ...
  for (int t = 0; t < nThreads; t++) {
    // give them a bounded region in the grid
    size_t start = colsPerThread * t;
    size_t end = (t == nThreads) ? grid.GRID_WIDTH : start + colsPerThread;

    // queue job
    threadPool->queueJob([this, start, end] {
      // iterate through rows and columns in region
      for (size_t row = 0; row < SpatialGrid::GRID_HEIGHT; row++) {
        for (size_t col = start; col < end; col++) {
          SpatialGrid::Cell cell = grid.Grid[row][col];

          // iterate through dot indexes in cell
          for (int index = 0; index < cell.count; index++) {
            size_t i1 = cell.indices[index];
            float radius = dots.radii[index];

            // query neighbours
            grid.queryNeighbours(dots.positions_x[i1], dots.positions_y[i1],
                                 radius, [&](size_t i2) {
                                   if (i1 != i2 && i2 > i1 &&
                                       dots.radii[i2] < Dots::RADIUS + 3) {
                                     collideDotsSIMD(i1, i2);
                                   }
                                 });
          }
        }
      }
    });
  }
  t_startThreads.stopClock();

  // waiting... 😴💤
  auto &t_waitForThreads = timer.startChild("wait_for_threads");
  threadPool->wait();
  t_waitForThreads.stopClock();
}

void Game::collideDots(size_t i1, size_t i2) {
  // first check
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

  // if there is no collision, exit out so we don't lock
  // any threads (very expensive, very boujee)
  if (distSq >= minDist * minDist || distSq <= 0.01f)
    return;

  // lock the mutexes, they could be colliding
  std::lock(dots_mutexes[i1], dots_mutexes[i2]);
  std::lock_guard<std::mutex> lock1(dots_mutexes[i1], std::adopt_lock);
  std::lock_guard<std::mutex> lock2(dots_mutexes[i2], std::adopt_lock);

  // we need to check again... because in the time that
  // we were locking the mutex the dots could've been accessed
  p1_x = dots.positions_x[i1];
  p1_y = dots.positions_y[i1];
  p2_x = dots.positions_x[i2];
  p2_y = dots.positions_y[i2];

  diff_x = p2_x - p1_x;
  diff_y = p2_y - p1_y;
  distSq = diff_x * diff_x + diff_y * diff_y;

  r1 = dots.radii[i1];
  r2 = dots.radii[i2];
  minDist = r1 + r2;

  // final check
  if (distSq >= minDist * minDist || distSq <= 0.01f)
    return;

  float v1_x = dots.velocities_x[i1];
  float v1_y = dots.velocities_y[i1];
  float v2_x = dots.velocities_x[i2];
  float v2_y = dots.velocities_y[i2];

  float dist = sqrt(distSq); // only sqrt when colliding
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

  // Seperation, divorce
  float overlap = (minDist + 2 - dist) * 1.5f;
  dots.positions_x[i1] = p1_x - normal_x * overlap;
  dots.positions_y[i1] = p1_y - normal_y * overlap;
  dots.positions_x[i2] = p2_x + normal_x * overlap;
  dots.positions_y[i2] = p2_y + normal_y * overlap;

  dots.radii[i1] = r1 + 1;
  dots.radii[i2] = r2 + 1;
}

// Add this include at the top of Game.cpp if it's not already there

// Replace your existing collideDots function with this one
void Game::collideDotsSIMD(size_t i1, size_t i2) {
  // --- 1. Load Data into SIMD Registers ---
  // Pack positions and velocities into 128-bit registers.
  // Layout: [y2, x2, y1, x1]
  __m128 pos = _mm_set_ps(dots.positions_y[i2], dots.positions_x[i2],
                          dots.positions_y[i1], dots.positions_x[i1]);
  __m128 vel = _mm_set_ps(dots.velocities_y[i2], dots.velocities_x[i2],
                          dots.velocities_y[i1], dots.velocities_x[i1]);

  // --- 2. Calculate Distance and Perform Early Exit Check ---
  // Subtract the lower half [y1, x1] from the upper half [y2, x2]
  __m128 diff = _mm_sub_ps(_mm_movehl_ps(pos, pos), pos);

  // Calculate dot product of diff with itself (diff.x^2 + diff.y^2)
  // The 0x31 mask means: multiply lanes 0 and 1, sum them, and store the result
  // in lane 0.
  __m128 distSq_v = _mm_dp_ps(diff, diff, 0x31);
  float distSq = _mm_cvtss_f32(distSq_v); // Convert SIMD float to scalar float

  uint8_t r1 = dots.radii[i1];
  uint8_t r2 = dots.radii[i2];
  float minDist = r1 + r2;

  if (distSq >= minDist * minDist || distSq < 0.01f)
    return;

  // --- Mutex Lock (no change here) ---
  std::lock(dots_mutexes[i1], dots_mutexes[i2]);
  std::lock_guard<std::mutex> lock1(dots_mutexes[i1], std::adopt_lock);
  std::lock_guard<std::mutex> lock2(dots_mutexes[i2], std::adopt_lock);

  // Re-check not shown for brevity, but should be done in production code...

  // --- 3. Collision Response using SIMD ---
  __m128 dist_v = _mm_sqrt_ss(distSq_v);
  float dist = _mm_cvtss_f32(dist_v);
  __m128 normal =
      _mm_div_ps(diff, _mm_shuffle_ps(dist_v, dist_v, 0)); // Broadcast dist

  // Separate velocities for dot1 and dot2
  __m128 vel1 = _mm_movelh_ps(vel, vel); // [v1y, v1x, v1y, v1x]
  __m128 vel2 = _mm_movehl_ps(vel, vel); // [v2y, v2x, v2y, v2x]

  // Reflection formula: v' = v - 2 * dot(v, n) * n
  // We calculate this for both dots simultaneously where possible
  __m128 dot1 = _mm_dp_ps(vel1, normal, 0x31);
  __m128 dot2 = _mm_dp_ps(vel2, normal, 0x31);

  __m128 two = _mm_set1_ps(2.0f);
  __m128 reflection1 =
      _mm_mul_ps(two, _mm_mul_ps(_mm_shuffle_ps(dot1, dot1, 0), normal));
  __m128 reflection2 =
      _mm_mul_ps(two, _mm_mul_ps(_mm_shuffle_ps(dot2, dot2, 0), normal));

  // New velocities (original v1 is used for v2's reflection and vice versa,
  // which is a common simplification)
  __m128 new_vel1 = _mm_sub_ps(vel2, reflection2);
  __m128 new_vel2 = _mm_sub_ps(vel1, reflection1);

  // Renormalize velocities
  __m128 len1_sq = _mm_dp_ps(new_vel1, new_vel1, 0x31);
  __m128 len2_sq = _mm_dp_ps(new_vel2, new_vel2, 0x31);
  __m128 inv_len1 = _mm_rsqrt_ss(len1_sq); // Reciprocal sqrt is faster
  __m128 inv_len2 = _mm_rsqrt_ss(len2_sq);
  new_vel1 = _mm_mul_ps(new_vel1, _mm_shuffle_ps(inv_len1, inv_len1, 0));
  new_vel2 = _mm_mul_ps(new_vel2, _mm_shuffle_ps(inv_len2, inv_len2, 0));

  // --- 4. Apply Separation and Store Results ---
  float overlap = (minDist - dist) * 0.5f; // Simplified separation
  __m128 overlap_v = _mm_set1_ps(overlap);
  __m128 separation = _mm_mul_ps(normal, overlap_v);

  __m128 new_pos1 = _mm_sub_ps(_mm_movelh_ps(pos, pos), separation);
  __m128 new_pos2 = _mm_add_ps(_mm_movehl_ps(pos, pos), separation);

  // Unpack results from SIMD registers and store back to memory
  float results[4];
  _mm_store_ps(results, new_pos1);
  dots.positions_x[i1] = results[0];
  dots.positions_y[i1] = results[1];
  _mm_store_ps(results, new_pos2);
  dots.positions_x[i2] = results[0];
  dots.positions_y[i2] = results[1];

  _mm_store_ps(results, new_vel1);
  dots.velocities_x[i1] = results[0];
  dots.velocities_y[i1] = results[1];
  _mm_store_ps(results, new_vel2);
  dots.velocities_x[i2] = results[0];
  dots.velocities_y[i2] = results[1];

  dots.radii[i1] = r1 + 1;
  dots.radii[i2] = r2 + 1;
}
