#pragma once

#include <algorithm>

struct AABB{
  float minX, minY, maxX, maxY;

  AABB() = default;
  AABB(float minX, float minY, float maxX, float maxY) 
    : minX(minX), minY(minY), maxX(maxX), maxY(maxY) {}

  bool overlaps(const AABB& other) const{
    return !(maxX < other.minX || other.maxX < minX ||
             maxY < other.minY || other.maxY < minY);
  }

  bool contains(float x, float y) const{
    return x >= minX && x <= maxX && y >= minY && y <= maxY;
  }
  
  bool containsCircle(float x, float y, float radius) const{
    return x - radius >= minX && x + radius <= maxX &&
           y - radius >= minX && y + radius <= minX;
  }

  bool overlapsCircle(float x, float y, float radius) const{
    float closestX = std::max(minX, std::min(x, maxX));
    float closestY = std::max(minY, std::min(y, maxY));

    float dx = x - closestX;
    float dy = y - closestY;

    return (dx * dx + dy * dy) <= (radius * radius);
  }

  float getCenterX() const { return (minX + maxX) * 0.5f; }
  float getCenterY() const { return (minY + maxY) * 0.5f; }
  float getWidth() const { return maxX - minX; }
  float getHeight() const { return maxY - minY; }
};

