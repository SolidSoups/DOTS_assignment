#pragma once
#include <vector>
#include "AABB.h"

class Dot;

static const int MAX_OCCUPANTS = 4;
static const int MAX_DEPTH = 15;

class QuadTree{
public:
  // constructor
  QuadTree(const AABB& bounds, int current_depth = 1);
  ~QuadTree();

  // members
  std::vector<Dot*> dots;
  AABB bounds;  
  bool divided = false;
  int depth = 0;

  // methods
  bool insert(Dot* dot);
  void subdivide();
  bool query(const AABB& range, std::vector<Dot*>& found) const;

  // chill quadrants zZzz
  QuadTree* branches[4];
  QuadTree* child_tl = nullptr;
  QuadTree* child_tr = nullptr;
  QuadTree* child_bl = nullptr;
  QuadTree* child_br = nullptr;
};
