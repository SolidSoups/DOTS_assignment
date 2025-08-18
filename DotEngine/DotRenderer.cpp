#include "DotRenderer.h"
#include "Dots.h"
#include "Settings.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>

DotRenderer::DotRenderer(SDL_Window *window)
    : m_sdlRenderer(nullptr), pixelBuffer(nullptr) {
  m_sdlRenderer = SDL_CreateRenderer(window, nullptr);
  if (!m_sdlRenderer)
    return;

  // init pixel buffer
  size_t bufferSize = Settings::SCREEN_WIDTH * Settings::SCREEN_HEIGHT;
  pixelBuffer = new (std::nothrow) uint32_t[bufferSize];
  // no safeguards
  memset(pixelBuffer, 0, bufferSize * sizeof(uint32_t));

  // init frame texture
  frameTexture = SDL_CreateTexture(
      m_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
      Settings::SCREEN_WIDTH, Settings::SCREEN_HEIGHT);

  // initialize circle cache
  for (int r = Dots::RADIUS; r <= Dots::RADIUS + 3; ++r) {
    CreateCircle(r);
  }
}

// may be naive, but its precomputed anyways
void DotRenderer::CreateCircle(int radius) {
  std::cout << "Creating circle" << "\n";
  CirclePixels newCircle;
  for (int x = -radius; x <= radius; x++) {
    for (int y = -radius; y <= radius; y++) {
      if ((x * x + y * y) <= radius * radius) { // inside the circle
        newCircle.offsets.push_back({x, y});
      }
    }
  }
  circleCache[radius] = newCircle;
}

DotRenderer::~DotRenderer() {
  if (m_sdlRenderer) {
    SDL_DestroyTexture(frameTexture);
    SDL_DestroyRenderer(m_sdlRenderer);
    m_sdlRenderer = nullptr;
  }
}

void DotRenderer::SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  if (m_sdlRenderer) {
    SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, a);
    red = r;
    green = g;
    blue = b;
    alpha = a;
  }
}

void DotRenderer::Clear() {
  if (m_sdlRenderer) {
    memset(pixelBuffer, 0,
           Settings::SCREEN_WIDTH * Settings::SCREEN_HEIGHT * sizeof(uint32_t));
    SDL_RenderClear(m_sdlRenderer);
  }
}

void DotRenderer::Present() {
  if (m_sdlRenderer) {
    SDL_UpdateTexture(frameTexture, nullptr, pixelBuffer,
                      Settings::SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderTexture(m_sdlRenderer, frameTexture, nullptr, nullptr);

    SDL_RenderPresent(m_sdlRenderer);
  }
}

void DotRenderer::DrawPoint(int x, int y) {
  if (m_sdlRenderer) {
    SDL_RenderPoint(m_sdlRenderer, x, y);
  }
}

void DotRenderer::DrawFilledCircle(int centerX, int centerY, int radius, int r,
                                   int b, int g, int a) {
  if (!m_sdlRenderer)
    return;

  if(radius > Dots::RADIUS + 2)
    return;

  if (circleCache.find(radius) == circleCache.end()) {
    CreateCircle(radius);
  }

  // lets just do white
  uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;

  const auto &offsets = circleCache[radius].offsets;
  for (const auto &[dx, dy] : offsets) {
    int absX = centerX + dx;
    int absY = centerY + dy;

    if (absX >= 0 && absX < Settings::SCREEN_WIDTH && absY >= 0 &&
        absY < Settings::SCREEN_HEIGHT) {
      size_t bufferIndex = absX + absY * Settings::SCREEN_WIDTH;

      pixelBuffer[bufferIndex] = color;
    }
  }
}

void DotRenderer::DrawRect(float minX, float minY, float maxX, float maxY) {
  if (!m_sdlRenderer)
    return;

  SDL_RenderLine(m_sdlRenderer, minX, minY, maxX, minY); // top
  SDL_RenderLine(m_sdlRenderer, maxX, minY, maxX, maxY); // right
  SDL_RenderLine(m_sdlRenderer, minX, maxY, maxX, maxY); // bottom
  SDL_RenderLine(m_sdlRenderer, minX, minY, minX, maxY); // top
}

void DotRenderer::RenderTexture(SDL_Texture *texture, const SDL_FRect *srcRect,
                                const SDL_FRect *dstRect) {
  if (m_sdlRenderer && texture) {
    SDL_RenderTexture(m_sdlRenderer, texture, srcRect, dstRect);
  }
}
