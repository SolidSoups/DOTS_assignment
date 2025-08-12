#pragma once
#include <vector>
#include "AABB.h"

class DotRenderer;
class QuadTree;
class Dot;

class Game
{
public:
	Game(DotRenderer* aRenderer);
  ~Game();
	void Update(float aDeltaTime);
  void processCollisions();
  void collideDots(Dot* d1, Dot* d2);
	void CleanUp();
  void createQuadTree();
private:
  float timeSinceUpdate;
  std::vector<Dot*> dots;

private:
  const int dot_amount;
  const int screen_height;
  const int screen_width;
  const int quad_refresh_rate_millis;

	DotRenderer* renderer; // self managed

  // Quad stuff
  const AABB default_bounds;
  QuadTree* quadTree;
};
