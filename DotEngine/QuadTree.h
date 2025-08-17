#pragma once

// includes
#include "AABB.h"
#include "Dots.h"
#include "SimpleProfiler.h"

// std
#include <memory>
#include <vector>

class QuadTreeNode {
public:
  static const int MAX_OBJECTS = 32;
  static const int MAX_LEVELS = 4;

  //tmp
  mutable size_t query_visits = 0;
  mutable size_t query_bounds_rejected = 0;
  mutable size_t query_leafs_checks = 0;

private:
  int level;
  AABB bounds;
  std::vector<size_t> dots_indices;
  bool divided;

  // Children: [0] = NW, [1] = NE, [2] = SW, [3] = SE
  std::unique_ptr<QuadTreeNode> branches[4];

public:
  QuadTreeNode(int level, const AABB &bounds)
      : level(level), bounds(bounds), divided(false) {
    dots_indices.reserve(MAX_OBJECTS + 1);
  }

  ~QuadTreeNode() = default; // TODO: what is this

  bool insert(size_t index, const Dots& dots) {

    // if dot doesnt fit inside this node's bounds, ignore it
    if (!bounds.contains(dots.positions_x[index], dots.positions_y[index])) {
      return false;
    }

    // if we have room and aren't divided, add it here
    if (!divided && (level == MAX_LEVELS || dots_indices.size() <= MAX_OBJECTS)) {
      dots_indices.push_back(index);
      return true;
    }

    // At this point we need to subdivide
    if (!divided) {
      subdivide(dots);
    }

    // Try to insert into children
    for (size_t i = 0; i < 4; i++) {
      if(branches[i]->insert(index, dots))
        break;
    }

    return true;
  }

  // Query dots in a given range, now with zero allocations
  template <typename Callback>
  void query(const AABB &range, Callback callback) const {
    query_visits++;

    // Early escape if we don't overlap the range
    if (!bounds.overlaps(range)) {
      query_bounds_rejected++;
      return;
    }

    // If not divided, chekk all dots in this node
    if (!divided) {
      query_leafs_checks++;
      for (size_t dot_index : dots_indices) {
        callback(dot_index); // its faster not to check range here (+10fps)
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
    dots_indices.clear();
    divided = false;
    for (int i = 0; i < 4; i++) {
      branches[i].reset();
    }
  }

  int size() const {
    int count = dots_indices.size();
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
  void subdivide(const Dots& dots) {
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
    std::vector<size_t> oldDots_indices = std::move(dots_indices);
    dots_indices.clear();

    for (size_t dot_index : oldDots_indices) {
      // Try to insert into children
      for (size_t i = 0; i < 4; i++) {
        branches[i]->insert(dot_index, dots);
      }
    }
  }

public: //tmp
  void resetStats() const{
    query_visits = 0;
    query_bounds_rejected = 0;
    query_leafs_checks = 0;
    if(divided){
      for(int i=0; i<4; i++){
        if(branches[i]) branches[i]->resetStats();
      }
    }
  }

  void collectStats(size_t& visits, size_t& rejects, size_t& checks) const{
    visits += query_visits;
    rejects += query_bounds_rejected;
    checks += query_leafs_checks;
    if(divided){
      for(int i=0; i<4;i++){
        if(branches[i]) branches[i]->collectStats(visits, rejects, checks);
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

  void insert(size_t dot_index, const Dots& dots) {
    if (root) {
      root->insert(dot_index, dots);
    }
  }

  template <typename Callback>
  void query(const AABB &range, Callback callback) const {
    if (root) {
      root->query(range, callback);
    }
  }

  /// delete and initialize root QuadTreeNode
  void clear() { root = std::make_unique<QuadTreeNode>(0, worldBounds); }

  int size() const { return root ? root->size() : 0; }

  const AABB &getBounds() const { return worldBounds; }

  void rebuild(const Dots& dots) {
    clear();
    for (size_t i=0; i<dots.size(); i++) {
      insert(i, dots);
    }
  }

  std::vector<AABB> getAllBounds(){
    if(root){
      std::vector<AABB> bounds; 
      root->getAllBounds(bounds);
      return bounds;
    }
    return {};
  }

  QuadTreeNode* getRoot() const { return root.get(); }
};
