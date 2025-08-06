#pragma once
#include <vector>
#include "AABB.h"

static const int SCREEN_WIDTH = 1000;
static const int SCREEN_HEIGHT = 600;
static const int QUAD_TIME_MILLIS = 500;
static const int RANGE_CHECK_SIZE = 10;

class DotRenderer;
class QuadTree;
class Dot;

class Game
{
public:
	Game(DotRenderer* aRenderer, const int dots);
	void Update(float aDeltaTime);
	void CleanUp();
  void createQuadTree();
private:
  float timeSinceUpdate;
  std::vector<Dot*> dots;
  const int dot_amount = 10;


	DotRenderer* renderer;

  // Quad stuff
  const AABB default_bounds;
  QuadTree* quadTree;
};
