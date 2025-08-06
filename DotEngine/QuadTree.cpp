#include "QuadTree.h"
#include "Dot.h"

// std
#include <vector>

QuadTree::QuadTree(const AABB& bounds)
    : bounds(bounds), divided(false) {
  // set all childs to null
  branches[0] = nullptr;
  branches[1] = nullptr;
  branches[2] = nullptr;
  branches[3] = nullptr;
}

QuadTree::~QuadTree() {
  if(!divided)
    return;
  for(int i=0; i<4; i++){
    if(branches[i] == nullptr)
      continue;
    delete branches[i];
    branches[i] = nullptr;
  }
}

bool QuadTree::insert(Dot *dot) {
  if (!bounds.contains(dot->position))
    return false;

  // normal insertion if occupancy is small
  if (dots.size() < MAX_OCCUPANTS) {
    dots.push_back(dot);
    return true;
  }

  // divide if we ain't divided yet
  if (!divided)
    subdivide();

  // pass insertion to child branches
  for(int i=0; i<4; i++){
    if(branches[i] && branches[i]->insert(dot))
      return true;
  }

  return false;
}

void QuadTree::subdivide() {
  // create bounds
  float half_x = bounds.size.x / 2.f;
  float half_y = bounds.size.y / 2.f;
  AABB new_bounds[] = {
    {bounds.pos.x, bounds.pos.y, half_x, half_y},
    {bounds.pos.x + half_x, bounds.pos.y, half_x, half_y},
    {bounds.pos.x, bounds.pos.y + half_y, half_x, half_y},
    {bounds.pos.x + half_x, bounds.pos.y + half_y, half_x, half_y}
  };

  // add dots to branches they belong, ignore creating 
  // branches that contain no dots
  for (auto dot : dots) {
    if(!dot)
      continue;

    for(int i=0; i<4; i++){
      if(!new_bounds[i].contains(dot->position))
        continue;

      if(branches[i] == nullptr)
        branches[i] = new QuadTree(new_bounds[i]);

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
    for(Dot* dot : dots){
      if(dot && range.contains(dot->position)){
        found.push_back(dot); 
        any_found = true;
      }
    }
  }
  else {
    // query all children
    for(int i=0; i<4; i++){
      if(branches[i])
        any_found = branches[i]->query(range, found);
    }
  }

  return any_found;
}
