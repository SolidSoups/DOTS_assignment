#pragma once
#include <vector>
#include <memory>
#include "AABB.h"
#include "Dots.h"
#include "SpatialGrid.h"
#include "SimpleProfiler.h"


class DotRenderer;
class QuadTree;
class ThreadPool;

class Game
{
public:
	Game(DotRenderer* aRenderer, ThreadPool* threadPool, Timer& timer);
  ~Game();
	void Update(float aDeltaTime);
  void processCollisions();
  void collideDots(size_t i1, size_t i2);
private:
  float timeSinceUpdate;
  /// Owner: Game
  Dots dots;

private:
	DotRenderer* renderer; // self managed
  Timer& timer;
  ThreadPool* threadPool;
  SpatialGrid grid;
};
