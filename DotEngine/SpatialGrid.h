#pragma once
#include "Dots.h"
#include "Settings.h"

// std
#include <cstdint>
#include <cstdio>
#include <cstring>

class SpatialGrid {
private:
  static constexpr int GRID_WIDTH = 80;
  static constexpr int GRID_HEIGHT = 45;
  static constexpr int CELL_CAPACITY = 64;

  struct Cell {
    size_t indices[CELL_CAPACITY];
    int count = 0;
  };

  Cell grid[GRID_WIDTH][GRID_HEIGHT];
  float cell_width;
  float cell_height;

public:
  SpatialGrid() {
    cell_width = Settings::SCREEN_WIDTH / float(GRID_WIDTH);
    cell_height = Settings::SCREEN_HEIGHT / float(GRID_HEIGHT);
  }

  void clear() { memset(grid, 0, sizeof(grid)); }

  void rebuild(const Dots &dots) {
    clear();

    for (size_t i = 0; i < dots.size(); i++) {
      // skip dead dots
      if (dots.radii[i] >= Dots::RADIUS + 3)
        continue;

      int gx = static_cast<int>(dots.positions_x[i] / cell_width);
      int gy = static_cast<int>(dots.positions_y[i] / cell_height);

      // bounds check
      if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
        Cell &cell = grid[gy][gx];
        if (cell.count < CELL_CAPACITY) {
          cell.indices[cell.count++] = static_cast<uint16_t>(i);
        }
      }
    }
  }

  template <typename Callback>
  void queryNeighbours(float x, float y, float radius, Callback cb) const {
    int min_gx = std::max(0, static_cast<int>((x - radius) / cell_width));
    int max_gx = std::min(GRID_WIDTH, static_cast<int>((x + radius) / cell_width));
    int min_gy = std::max(0, static_cast<int>((y - radius) / cell_height));
    int max_gy = std::min(GRID_HEIGHT, static_cast<int>((y + radius) / cell_height));

    for(int gy = min_gy; gy <= max_gy; gy++){
      for(int gx = min_gx; gx <= max_gx; gx++){
        const Cell& cell = grid[gy][gx];
        for(size_t i = 0; i < cell.count; i++){
          cb(cell.indices[i]);
        }
      }
    }
  }

  // Debug stats
  size_t getOccupiedCells() const {
    size_t occupied = 0;
    for (int y = 0; y < GRID_HEIGHT; y++) {
      for (int x = 0; x < GRID_WIDTH; x++) {
        if (grid[y][x].count > 0) occupied++;
      }
    }
    return occupied;
  }

  float getAverageDotsPerCell() const {
    size_t total_dots = 0;
    size_t occupied_cells = 0;
    for (int y = 0; y < GRID_HEIGHT; y++) {
      for (int x = 0; x < GRID_WIDTH; x++) {
        if (grid[y][x].count > 0) {
          total_dots += grid[y][x].count;
          occupied_cells++;
        }
      }
    }
    return occupied_cells > 0 ? float(total_dots) / occupied_cells : 0;
  }
};
