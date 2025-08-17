#pragma once

// includes
#include "AABB.h"
#include "Dot.h"

// std
#include <memory>
#include <vector>

class QuadTreeNode {
public:
  static const int MAX_OBJECTS = 8;
  static const int MAX_LEVELS = 9;

private:
  int level;
  AABB bounds;
  std::vector<Dot *> dots;
  bool divided;

  // Children: [0] = NW, [1] = NE, [2] = SW, [3] = SE
  std::unique_ptr<QuadTreeNode> branches[4];

public:
  QuadTreeNode(int level, const AABB &bounds)
      : level(level), bounds(bounds), divided(false) {
    dots.reserve(MAX_OBJECTS + 1);
  }

  ~QuadTreeNode() = default; // TODO: what is this

  bool insert(Dot *dot) {
    // if dot doesnt fit inside this node's bounds, ignore it
    if (!bounds.contains(dot->position.x, dot->position.y)) {
      return false;
    }

    // if we have room and aren't divided, add it here
    if (!divided && (level == MAX_LEVELS || dots.size() <= MAX_OBJECTS)) {
      dots.push_back(dot);
      return true;
    }

    // At this point we need to subdivide
    if (!divided) {
      subdivide();
    }

    // Try to insert into children
    for (size_t i = 0; i < 4; i++) {
      if(branches[i]->insert(dot))
        break;
    }

    return true;
  }

  // Query dots in a given range, now with zero allocations
  template <typename Callback>
  void query(const AABB &range, Callback callback) const {
    // Early escape if we don't overlap the range
    if (!bounds.overlaps(range)) {
      return;
    }

    // If not divided, chekk all dots in this node
    if (!divided) {
      for (Dot *dot : dots) {
        callback(dot); // its faster not to check range here (+10fps)
      }
    } else {
      // Query all children
      for (int i = 0; i < 4; i++) {
        if (branches[i]) {
          branches[i]->query(range, callback);
        }
      }
    }
  }

  void clear() {
    dots.clear();
    divided = false;
    for (int i = 0; i < 4; i++) {
      branches[i].reset();
    }
  }

  int size() const {
    int count = dots.size();
    if (divided) {
      for (int i = 0; i < 4; i++) {
        if (branches[i]) {
          count += branches[i]->size();
        }
      }
    }
    return count;
  }

  const AABB &getBounds() const { return bounds; }

  void getAllBounds(std::vector<AABB>& allBounds) const {
    if(!divided){
      allBounds.push_back(bounds);
    }
    else{
      for(int i=0; i<4; i++){
        if(branches[i]){
          branches[i]->getAllBounds(allBounds);
        }
      }
    }
  }

private:
  void subdivide() {
    if (divided || level >= MAX_LEVELS) {
      return;
    }

    float cX = bounds.getCenterX();
    float cY = bounds.getCenterY();

    // Create four children
    branches[0] = std::make_unique<QuadTreeNode>(
        level + 1, AABB(bounds.minX, bounds.minY, cX, cY));
    branches[1] = std::make_unique<QuadTreeNode>(
        level + 1, AABB(cX, bounds.minY, bounds.maxX, cY));
    branches[2] = std::make_unique<QuadTreeNode>(
        level + 1, AABB(bounds.minX, cY, cX, bounds.maxY));
    branches[3] = std::make_unique<QuadTreeNode>(
        level + 1, AABB(cX, cY, bounds.maxX, bounds.maxY));

    divided = true;

    // Redistribute existing dots to children
    std::vector<Dot *> oldDots = std::move(dots);
    dots.clear();

    for (Dot *dot : oldDots) {
      // Try to insert into children
      for (size_t i = 0; i < 4; i++) {
        branches[i]->insert(dot);
      }
    }
  }
};



class QuadTree {
private:
  std::unique_ptr<QuadTreeNode> root;
  AABB worldBounds;

public:
  QuadTree(const AABB &bounds) : worldBounds(bounds) {
    root = std::make_unique<QuadTreeNode>(0, bounds);
  }

  ~QuadTree() = default;

  void insert(Dot *dot) {
    if (root) {
      root->insert(dot);
    }
  }

  template <typename Callback>
  void query(const AABB &range, Callback callback) const {
    if (root) {
      root->query(range, callback);
    }
  }

  void clear() { root = std::make_unique<QuadTreeNode>(0, worldBounds); }

  int size() const { return root ? root->size() : 0; }

  const AABB &getBounds() const { return worldBounds; }

  void rebuild(const std::vector<Dot *> &dots) {
    clear();
    for (Dot *dot : dots) {
      insert(dot);
    }
  }

  std::vector<AABB> getAllBounds(){
    if(root){
      std::vector<AABB> bounds; 
      root->getAllBounds(bounds);
      return bounds;
    }
  }
};
