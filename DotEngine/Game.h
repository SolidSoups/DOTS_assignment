#pragma once
#include <vector>
#include <memory>
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
	DotRenderer* renderer; // self managed
  std::unique_ptr<QuadTree> quadTree;
};
