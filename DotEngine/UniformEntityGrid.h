#pragma once
#include "glm/glm.hpp"

// std
#include <utility>
#include <algorithm>
#include <vector>

class Dot;

// Switching to a uniform grid is a lot more advantagous
// when the amount of dots approaches the point where the screen
// is completely filled. Using a QuadTree in this scenario means
// the branches will ALWAYS be fully extended and retrieval will
// cause unnecessary operations. QuadTree's are more versatile
// when it comes to optimizing retrieval of spaced out entities.
//
// Even a Spatial Hash Grid would be great, except it excels at
// large infinite worlds, but in our case we can know the exact
// dimensions of the world before runtime.

struct GridCell{
  std::vector<Dot*> bucket;
  GridCell(){
    bucket.reserve(5);
  }

  void add(Dot* dot){
    bucket.push_back(dot); 
  }

  void remove(Dot* dot){
    // extremely faster removal with pop&swap
    auto it = std::find(bucket.begin(), bucket.end(), dot);
    if(it != bucket.end()){
      std::swap(*it, bucket.back());
      bucket.pop_back();
    }
  }
};

class FastGrid {
public:
  FastGrid(int w, int h, int _size);
  void Init();

  void Insert(Dot * d);
  void UpdateDot(Dot* d);
  
  // old query
  std::vector<Dot*> QueryCells(glm::ivec2 gridPos);

  // new query
  template<typename Callback>
  void ForEachInQueryArea(glm::ivec2 gridPos, int radius, Callback callback){
    // How many cells does this radius span?
    int cellRadius = static_cast<int>(std::ceil(static_cast<float>(radius)/cellSize));

    int minX = std::max(0, gridPos.x - cellRadius);
    int maxX = std::min(bw-1, gridPos.x + cellRadius);
    int minY = std::max(0, gridPos.y - cellRadius);
    int maxY = std::min(bh-1, gridPos.y + cellRadius);

    for (int x = minX; x <= maxX; ++x){
      for (int y = minY; y <= maxY; ++y){
        const auto& cell = grid[getCellIndex(glm::ivec2{x, y})].bucket;
        for(Dot* dot : cell){
          if(!callback(dot)) return;
        }
      }
    }
  }

  // helpers
  glm::ivec2 getGridPos(Dot*);
  glm::ivec2 getGridPos(glm::vec2 pos);
  size_t getCellIndex(glm::ivec2 cell);
  size_t getCellIndex(glm::vec2 pos);

private:
  std::vector<GridCell> grid;
  const float dSize = 6.f;
  const int bw, bh; // border
  const float cellSize; // cell size
  const int iw, ih; // grid size
  const float colsPerPixel, rowsPerPixel;

};
