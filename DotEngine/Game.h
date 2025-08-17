#pragma once
#include <vector>
#include <memory>
#include "AABB.h"
#include "Dots.h"
#include "SpatialGrid.h"


class DotRenderer;
class QuadTree;

class Game
{
public:
	Game(DotRenderer* aRenderer);
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
  // std::unique_ptr<QuadTree> quadTree;
  SpatialGrid grid;
};
