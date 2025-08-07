#pragma once
#include <vector>
#include "AABB.h"

static const int SCREEN_WIDTH = 1400;
static const int SCREEN_HEIGHT = 800;
static const int QUAD_TIME_MILLIS = 10;
static const int RANGE_CHECK_SIZE = 20;

class DotRenderer;
class QuadTree;
class Dot;

class Game
{
public:
	Game(DotRenderer* aRenderer, const int dots);
  ~Game();
	void Update(float aDeltaTime);
  void processCollisions();
	void CleanUp();
  void createQuadTree();
private:
  float timeSinceUpdate;
  std::vector<Dot*> dots;
  const int dot_amount = 10;


	DotRenderer* renderer; // self managed

  // Quad stuff
  const AABB default_bounds;
  QuadTree* quadTree;
};
