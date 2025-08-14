#pragma once
#include "AABB.h"
#include "UniformEntityGrid.h"
#include <vector>
#include <unordered_map>

class DotRenderer;
class QuadTree;
class Dot;

struct CollisionPair{
  Dot* d1;
  Dot* d2;
};

class Game {
public:
  Game(DotRenderer *aRenderer);
  ~Game();
  void Update(float aDeltaTime);
  void processCollisions();
  void collectCollisions();
  void collideDots(Dot *d1, Dot *d2);
  void CleanUp();

private:
  std::vector<Dot *> dots;
  std::vector<CollisionPair> collisionPairs;

private:
  const int dot_amount;
  const int screen_height;
  const int screen_width;

  DotRenderer *renderer; // self managed

  FastGrid *grid;
};
