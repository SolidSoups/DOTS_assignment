#include "DotRenderer.h"
#include "Game.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>
#include <Debug.h>
#include <algorithm>
#include "FrameTime.h"
#include "DotSettings.h"

int main(int argc, char *args[]) {
  Debug::Log("PROGRAM START");

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    const char *err = SDL_GetError();
    Debug::LogError(err);
    return 1;
  }

  if (!TTF_Init()) {
    const char *err = SDL_GetError();
    Debug::LogError(err);
    SDL_Quit();
    return 1;
  }

  SDL_Window *window =
      SDL_CreateWindow("Game", globalSettings.SCREEN_WIDTH, globalSettings.SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

  DotRenderer *renderer = new DotRenderer(window);
  if (!renderer->GetSDLRenderer()) {
    const char *err = SDL_GetError();
    Debug::LogError(err);
    delete renderer;
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  TTF_Font *font = TTF_OpenFont("./fonts/arial.ttf", 24);
  if (font == nullptr) {
    const char *err = SDL_GetError();
    Debug::LogError(err);
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  

  renderer->SetDrawColor(0x00, 0x00, 0x00, 0xFF);

  Debug* debug = new Debug(renderer, font);
  Game *game = new Game(renderer);

  bool quit = false;
  SDL_Event e;

  Uint64 lastTick = SDL_GetPerformanceCounter();
  Uint64 currentTick;
  double deltaTime = 0;
  double fps = 0;
  int frameCount = 0;
  FrameTime frameTime(debug);
  int totalFrameCount = 0;
  double fpsAccumulator = 0.0;
  const double FPS_UPDATE_INTERVAL = 0.2f;

  // text debug
  std::string dotsCountText = "DOTS_AMOUNT: " + std::to_string(globalSettings.DOTS_AMOUNT);
  debug->UpdateScreenField("DOTS", dotsCountText);

  while (!quit) {

    currentTick = SDL_GetPerformanceCounter();
    deltaTime =
        (double)(currentTick - lastTick) / SDL_GetPerformanceFrequency();
    lastTick = currentTick;

    frameCount++;
    totalFrameCount++;
    fpsAccumulator += deltaTime;

    if (fpsAccumulator >= FPS_UPDATE_INTERVAL) {
      fps = frameCount / fpsAccumulator;

      frameCount = 0;
      fpsAccumulator = 0.0;
    }

    frameTime.Update(deltaTime);

    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
      case SDL_EVENT_QUIT:
        quit = true;
        break;
      case SDL_EVENT_KEY_DOWN:
        if (e.key.key == SDLK_ESCAPE)
          quit = true;
        break;
      }
    }

    renderer->SetDrawColor(0x00, 0x00, 0x00, 0xFF);
    renderer->Clear();

    game->Update(deltaTime);

    // - DEBUG
    std::string fpsText = "FPS: " + std::to_string(static_cast<int>(fps));
    debug->UpdateScreenField("fps", fpsText);
    std::string onePLow = "1% LOW: " + 
      std::to_string(static_cast<int>(frameTime.onepercentlow)) + "ms";
    debug->UpdateScreenField("opl", onePLow);
    debug->Render();

    renderer->Present();
  }

  delete game;
  delete renderer;
  TTF_CloseFont(font);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();

  return 0;
}
