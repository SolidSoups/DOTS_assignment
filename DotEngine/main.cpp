#include "DotRenderer.h"
#include "FrameTime.h"
#include "Game.h"
#include "Settings.h"
#include <Debug.h>
#include "SimpleProfiler.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include "Dots.h"
#include "ThreadPool.h"


int main() {
  Debug::Log("PROGRAM START");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
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
      SDL_CreateWindow("Game", Settings::SCREEN_WIDTH, Settings::SCREEN_HEIGHT,
                       SDL_WINDOW_OPENGL);

  ThreadPool* threadPool = new ThreadPool();

  SimpleProfiler* profiler = new SimpleProfiler();
  auto& totalClock = profiler->start("total");

  DotRenderer *renderer = new DotRenderer(window, threadPool, totalClock);

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

  Debug *debug = new Debug(renderer, font);
  Game *game = new Game(renderer, threadPool, totalClock);

  FrameTime frameTime;

  bool quit = false;
  SDL_Event e;

  Uint64 lastTick = SDL_GetPerformanceCounter();
  Uint64 currentTick;
  double deltaTime = 0;
  double fps = 0;
  int frameCount = 0;
  double fpsAccumulator = 0.0;
  const double FPS_UPDATE_INTERVAL = 0.2f;

  // text debug

  while (!quit) {
    currentTick = SDL_GetPerformanceCounter();
    deltaTime =
        (double)(currentTick - lastTick) / SDL_GetPerformanceFrequency();
    lastTick = currentTick;

    frameCount++;
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

    totalClock.startClock();
    renderer->SetDrawColor(0x00, 0x00, 0x00, 0xFF);
    renderer->Clear();

    game->Update(deltaTime);
    totalClock.stopClock();

    // - DEBUG
    std::string fpsText = "FPS: " + std::to_string(static_cast<int>(fps));
    debug->UpdateScreenField("fps", fpsText);
    std::string onePLow =
        "1% LOW: " + std::to_string(static_cast<int>(frameTime.onepercentlow)) +
        "ms";
    debug->UpdateScreenField("opl", onePLow);
    debug->Render();

    renderer->Present();

    static int pFrameCount=0;
    if(++pFrameCount % 60 == 0){
      profiler->reportTimersFull();
    }
  }

  Debug::OutputScreenFields();

  delete threadPool;
  delete game;
  delete renderer;
  TTF_CloseFont(font);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();

  return 0;
}
