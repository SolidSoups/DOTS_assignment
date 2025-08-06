#pragma once
#include "glm/glm.hpp"

struct AABB{
  AABB(float x, float y, float w, float h) : pos{x, y}, size{w, h} {}
  AABB(const AABB& other) : pos(other.pos), size(other.size){}
  glm::vec2 pos; // top-left corner
  glm::vec2 size; // full size to bottom-left corner

  bool contains(const glm::vec2& point) const{
    return !(
      point.x < pos.x ||
      point.y < pos.y ||
      point.x > pos.x + size.x ||
      point.y > pos.y + size.y
    );
  };
  
  bool overlaps(const AABB& other) const{
    return (
      (other.pos.x <= pos.x + size.x && other.pos.x + other.size.x >= pos.x) &&
      (other.pos.y <= pos.y + size.y && other.pos.y + other.size.y >= pos.y)
    );
  }
};

