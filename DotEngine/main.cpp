#include "DotRenderer.h"
#include "Game.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>

#define DEBUG_LOG
#ifdef DEBUG_LOG
#define log(msg) std::cout << msg << "\n";
#endif



int main(int argc, char *args[]) {
  log("PROGRAM START");

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    log("SDL_init failed!");
    return 1;
  }
  log("SDL_init success!");

  if (!TTF_Init()) {
    log("TTF init failed!");
    SDL_Quit();
    return 1;
  }
  log("TTF_init success!");

  SDL_Window *window =
      SDL_CreateWindow("Game", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

  DotRenderer *renderer = new DotRenderer(window);
  if (!renderer->GetSDLRenderer()) {
    log("DotRenderer->GetSDLRenderer() failed!");
    delete renderer;
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 1;
  }
  log("DotRenderer->GetSDLRenderer() success!");

  TTF_Font *font = TTF_OpenFont("./fonts/arial.ttf", 24);
  if (font == nullptr) {
    log("TTF_OpenFont failed!");
    const char *err = SDL_GetError();
    log(err);
    TTF_Quit();
    SDL_Quit();
    return 1;
  }
  log("TTF_OpenFont success!");

  renderer->SetDrawColor(0x00, 0x00, 0x00, 0xFF);

  Game *game = new Game(renderer, 500);
  log("Game Created!")

  bool quit = false;
  SDL_Event e;

  Uint64 lastTick = SDL_GetPerformanceCounter();
  Uint64 currentTick;
  double deltaTime = 0;
  double fps = 0;
  int frameCount = 0;
  double fpsAccumulator = 0.0;
  const double FPS_UPDATE_INTERVAL = 0.2f;

  while (!quit) {
    std::string frameoutput = "Frame: " + std::to_string(frameCount);
    log(frameoutput);
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

    // - FPS COUNTER -
    std::string fpsText = "FPS: " + std::to_string(static_cast<int>(fps));
    SDL_Surface *textSurface =
        TTF_RenderText_Solid(font, fpsText.c_str(), 0, {255, 255, 255, 255});
    if (textSurface != nullptr) {
      SDL_Texture *textTexture =
          SDL_CreateTextureFromSurface(renderer->GetSDLRenderer(), textSurface);
      if (textTexture != nullptr) {
        SDL_FRect renderQuad = {0, 0, (float)textSurface->w,
                                (float)textSurface->h};
        renderer->RenderTexture(textTexture, nullptr, &renderQuad);
        SDL_DestroyTexture(textTexture);
      }
      SDL_DestroySurface(textSurface);
    }
    // - FPS COUNTER -

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
