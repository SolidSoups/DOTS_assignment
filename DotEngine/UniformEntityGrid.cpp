#include "UniformEntityGrid.h"
#include "Debug.h"
#include "Dot.h"

// std
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

FastGrid::FastGrid(int w, int h, int _size) : 
  bw(w), 
  bh(h), 
  cellSize(_size * dSize), 
  iw(w / cellSize), 
  ih(h / cellSize),
  colsPerPixel(iw / w), rowsPerPixel(ih / h) 
{
  Init();
}

void FastGrid::Init() {
  // resize also creates new instances
  size_t sizeCap = iw * ih;
  grid.resize(sizeCap);

  // add some debug info
  std::string output = "GridSize: " + std::to_string(iw) + "x" + std::to_string(ih);
  Debug* idb = Debug::GetInstance();
  idb->UpdateScreenField("gridSize", output);
} 

void FastGrid::UpdateDot(Dot* dot){
  // erase from old bucket
  grid[dot->currentCellIndex].remove(dot);

  // add to new bucket
  size_t newIndex = getCellIndex(dot->position);
  dot->currentCellIndex = newIndex;
  grid[newIndex].add(dot);
}

void FastGrid::Insert(Dot *dot) {
  size_t index = getCellIndex(dot->position);
  dot->currentCellIndex = index;
  grid[index].add(dot);
}


std::vector<Dot *> FastGrid::QueryCells(glm::ivec2 gridPos) {
  std::vector<Dot *> foundDots;
  for (int x = std::max(0, gridPos.x - 1); x < gridPos.x + 2; ++x) {
    for (int y = std::max(0, gridPos.y - 1); y < gridPos.y + 2; ++y) {
      // upper bounds check
      if (x < bw && y < bh) {
        // do query, concatenate results
        size_t gridIndex = getCellIndex(glm::ivec2{x, y});
        std::vector<Dot *> cell = grid[gridIndex].bucket;
        foundDots.insert(foundDots.end(), cell.begin(), cell.end());
      }
    }
  }
  return foundDots;
}

glm::ivec2 FastGrid::getGridPos(Dot *dot) {
  return getGridPos(dot->position); 
}
glm::ivec2 FastGrid::getGridPos(glm::vec2 pos){
  return {
    (int)(pos.x * colsPerPixel),
    (int)(pos.y * rowsPerPixel)
  };
}

size_t FastGrid::getCellIndex(glm::ivec2 cell) {
  size_t index = cell.x * cell.y;
  if (index >= iw * ih) {
    Debug::LogError("Index overflow in indexFunc");
  }

  return index;
}
size_t FastGrid::getCellIndex(glm::vec2 pos) {
  glm::ivec2 gridPos = getGridPos(pos);
  size_t index = gridPos.x * gridPos.y;
  if (index >= iw * ih) {
    Debug::LogError("Index overflow in indexFunc");
  }

  return index;
}
