#pragma once

struct Settings{
  const int SCREEN_WIDTH;
  const int SCREEN_HEIGHT;

  const int QUAD_TREE_MILLIS_REFRESH_TIME; 
  const int QUAD_TREE_MAX_OCCUPANTS;
  const int QUAD_TREE_MAX_DEPTH;

  const int DOTS_AMOUNT;
  const int DOT_RADIUS;
  const float DOT_VELOCITY;
};

extern Settings globalSettings;
