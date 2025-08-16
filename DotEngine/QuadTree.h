#pragma once
#include "AABB.h"
#include <vector>
#include "Dot.h"

class Dot;

class QuadTree {
public:
  // constructor
  QuadTree(const AABB &bounds, int current_depth = 1);
  ~QuadTree();

  const int max_depth;
  const int max_occ;

  // members
  std::vector<Dot *> dots;
  AABB bounds;
  bool divided = false;
  int depth = 0;

  // methods
  bool insert(Dot *dot);
  void subdivide();
  bool query(const AABB &range, std::vector<Dot *> &found) const;

  template <typename Callback>
  void ForEachQueryArea(const AABB &range, Callback callback) {
    if (!bounds.overlaps(range))
      return;

    if (!divided) {
      for (Dot *dot : dots) {
          callback(dot);
      }
    } else {
      if (branches[0]) branches[0]->ForEachQueryArea(range, callback);
      if (branches[1]) branches[1]->ForEachQueryArea(range, callback);
      if (branches[2]) branches[2]->ForEachQueryArea(range, callback);
      if (branches[3]) branches[3]->ForEachQueryArea(range, callback);
    }
  }

  // chill quadrants zZzz
  QuadTree *branches[4];
};
