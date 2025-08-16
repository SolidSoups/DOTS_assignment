#include "QuadTree.h"
#include "Dot.h"
#include "Settings.h"

// std
#include <algorithm>
#include <queue>
#include <utility>
#include <vector>

QuadTree::QuadTree(const AABB &bounds, int current_depth)
    : bounds(bounds), divided(false),
      max_depth(Settings::QUAD_TREE_MAX_DEPTH),
      max_occ(Settings::QUAD_TREE_MAX_OCCUPANTS) {
  // set all childs to null
  branches[0] = nullptr;
  branches[1] = nullptr;
  branches[2] = nullptr;
  branches[3] = nullptr;
  depth = current_depth;
}

QuadTree::~QuadTree() {
  if (!divided)
    return;
  for (int i = 0; i < 4; i++) {
    if (branches[i] == nullptr)
      continue;
    delete branches[i];
    branches[i] = nullptr;
  }
}

bool QuadTree::insert(Dot *dot) {
  if (!bounds.overlaps({dot->position.x, dot->position.y, dot->radius * 2.f,
                        dot->radius * 2.f})) {
    return false;
  }

  // normal insertion if occupancy is small, or if max depth is reached
  if (dots.size() < max_occ || depth == max_depth) {
    dots.push_back(dot);
    return true;
  }

  // divide if we ain't divided yet
  if (!divided)
    subdivide();

  // pass insertion to child branches
  bool b_inserted = false;
  for (int i = 0; i < 4; i++) {
    if (branches[i] && branches[i]->insert(dot))
      b_inserted = true;
  }

  return b_inserted;
}

void QuadTree::subdivide() {
  // create bounds
  float half_x = bounds.size.x / 2.f;
  float half_y = bounds.size.y / 2.f;
  AABB new_bounds[] = {
      {bounds.pos.x, bounds.pos.y, half_x, half_y},
      {bounds.pos.x + half_x, bounds.pos.y, half_x, half_y},
      {bounds.pos.x, bounds.pos.y + half_y, half_x, half_y},
      {bounds.pos.x + half_x, bounds.pos.y + half_y, half_x, half_y}};

  // add dots to branches they belong, ignore creating
  // branches that contain no dots
  for (auto dot : dots) {
    if (!dot)
      continue;

    for (int i = 0; i < 4; i++) {
      if (!new_bounds[i].overlaps({dot->position.x - dot->radius,
                                   dot->position.y - dot->radius,
                                   dot->radius * 2.f, dot->radius * 2.f}))
        continue;

      if (branches[i] == nullptr)
        branches[i] = new QuadTree(new_bounds[i], depth + 1);

      branches[i]->insert(dot);
    }
  }

  dots.clear();
  divided = true;
}


bool QuadTree::query(const AABB &range, std::vector<Dot *> &found) const {
  // early-escape if we don't overlap the range
  if (!bounds.overlaps(range))
    return false;

  bool any_found = false;
  // combine both vectors if divided
  if (!divided) {
    // add found to range
    for (Dot *dot : dots) {
      if (dot && range.contains(dot->position)) {
        found.push_back(dot);
        any_found = true;
      }
    }
  } else {
    // query all children
    for (int i = 0; i < 4; i++) {
      if (branches[i])
        any_found = branches[i]->query(range, found);
    }
  }

  return any_found;
}
