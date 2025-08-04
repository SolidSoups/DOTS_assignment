#pragma once
#include "vector"

static const int SCREEN_WIDTH = 1000;
static const int SCREEN_HEIGHT = 800;

class DotRenderer;

class Game
{
public:
	Game(DotRenderer* aRenderer);
	void Update(float aDeltaTime);
	void CleanUp();
private:
	DotRenderer* renderer;
};

