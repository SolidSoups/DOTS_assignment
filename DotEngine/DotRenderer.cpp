#include "DotRenderer.h"
#include "Dots.h"
#include "Settings.h"
#include "SimpleProfiler.h"
#include "ThreadPool.h"

// lib
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

// std
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>

DotRenderer::DotRenderer(SDL_Window *window, ThreadPool *threadPool, Timer& timer)
    : m_sdlRenderer(nullptr), m_threadPool(threadPool), timer(timer),
      bufferSize(Settings::SCREEN_WIDTH * Settings::SCREEN_HEIGHT) {
  m_sdlRenderer = SDL_CreateRenderer(window, nullptr);
  if (!m_sdlRenderer)
    return;

  // initialize pixel bufffer modificants
  const size_t nThreads = threadPool->num_threads;
  m_threadSortedPixelData.resize(nThreads);
  for (int i = 0; i < nThreads; i++) {
    m_threadSortedPixelData[i].resize(nThreads);
  }
  m_combinedPixelBuffer = new (std::nothrow) uint32_t[bufferSize];

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
  delete[] m_combinedPixelBuffer;

  if (m_sdlRenderer) {
    SDL_DestroyTexture(frameTexture);
    SDL_DestroyRenderer(m_sdlRenderer);
    m_sdlRenderer = nullptr;
  }
}

void DotRenderer::SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  if (m_sdlRenderer) {
    SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, a);
  }
}

void DotRenderer::Clear() {
  if (m_sdlRenderer) {
    SDL_RenderClear(m_sdlRenderer);
  }
}

void DotRenderer::Present() {
  if (m_sdlRenderer) {
    SDL_RenderPresent(m_sdlRenderer);
  }
}

void DotRenderer::DrawPoint(int x, int y) {
  if (m_sdlRenderer) {
    SDL_RenderPoint(m_sdlRenderer, x, y);
  }
}

void DotRenderer::BatchDrawCirclesCPUThreaded(float pos_x[], float pos_y[],
                                              uint8_t radii[], int red[],
                                              size_t size) {
  if (!m_sdlRenderer || size == 0)
    return;

  auto& t_total = timer.startChild("render_total");

  std::vector<std::thread> workers;
  const int indicesPerThread = size / MAX_THREADS;

  const int nThreads = m_threadPool->num_threads;

  // queue work for threads
  auto& t_drawBuffer = t_total.startChild("drawing_buffers");
  auto& t_drawBuffer_queueJobs = t_drawBuffer.startChild("queuing_jobs");
  for (size_t t = 0; t < nThreads; ++t) {
    // we need to know what indices to work on
    size_t start = t * indicesPerThread;
    size_t end = (t == nThreads - 1) ? size - 1 : start + indicesPerThread;

    // clear the old pixel data
    for (auto &bin : m_threadSortedPixelData[t]) {
      bin.clear();
    }

    m_threadPool->queueJob([this, pos_x, pos_y, radii, red, t, start, end]() {
      for (size_t i = start; i <= end; i++) {
        DrawFilledCircle(pos_x[i], pos_y[i], radii[i], red[i], 125, 125, 255,
                         t);
      }
    });
  }
  t_drawBuffer_queueJobs.stopClock();

  auto& t_drawBuffer_waitForThreads = t_drawBuffer.startChild("wait_for_threads");
  m_threadPool->wait();
  t_drawBuffer_waitForThreads.stopClock();
  t_drawBuffer.stopClock();

  // combine pixelBuffers
  CombineThreadBuffers(m_threadSortedPixelData, m_combinedPixelBuffer, t_total);

  // update and render texture
  auto& t_sdlCalls = t_total.startChild("sdl_calls");
  SDL_UpdateTexture(frameTexture, nullptr, m_combinedPixelBuffer,
                    Settings::SCREEN_WIDTH * sizeof(uint32_t));
  SDL_RenderTexture(m_sdlRenderer, frameTexture, nullptr, nullptr);
  t_sdlCalls.stopClock();
  t_total.stopClock();
}

// const size_t indicesPerThread = pixelData.size() / m_threadPool->num_threads;
void DotRenderer::CombineThreadBuffers(
    const std::vector<std::vector<std::vector<Pixel>>> &pixelData,
    uint32_t *outputBuffer, Timer& t_total) {
  auto& t_combineBuffers = t_total.startChild("combine_buffers");

  memset(outputBuffer, 0, bufferSize * sizeof(uint32_t));

  const int maxThreads = m_threadPool->num_threads;

  auto& t_combineBuffers_startThreads = t_combineBuffers.startChild("start_threads");
  for (int t = 0; t < maxThreads; ++t) {
    m_threadPool->queueJob([this, &pixelData, outputBuffer, t] {
      // This combining thread (t) is reponsible for one region.
      // It must process all bins for its region from ALL drawing threads
      for (int i = 0; i < pixelData.size(); i++) {
        for (const auto &pixel : pixelData[i][t]) {
          outputBuffer[pixel.index] =
              BlendAdditive(pixel.color, outputBuffer[pixel.index]);
        }
      }
    });
  }
  t_combineBuffers_startThreads.stopClock();

  auto& t_combineBuffers_waitForThreads = t_combineBuffers.startChild("wait_for_threads");
  m_threadPool->wait();
  t_combineBuffers_waitForThreads.stopClock();

  t_combineBuffers.stopClock();
}

// no support for alpha
uint32_t DotRenderer::BlendAdditive(uint32_t src, uint32_t dst) {
  // extract rgb
  uint32_t srcR = (src >> 16) & 0xFF;
  uint32_t srcG = (src >> 8) & 0xFF;
  uint32_t srcB = src & 0xFF;

  uint32_t dstR = (dst >> 16) & 0xFF;
  uint32_t dstG = (dst >> 8) & 0xFF;
  uint32_t dstB = dst & 0xFF;

  // add with saturation
  uint32_t r = std::min(255U, srcR + dstR);
  uint32_t g = std::min(255U, srcG + dstG);
  uint32_t b = std::min(255U, srcB + dstB);

  return 0xFF000000 | (r << 16) | (g << 8) | b; // Full alpha
}

void DotRenderer::DrawFilledCircle(int centerX, int centerY, int radius, int r,
                                   int b, int g, int a, int drawingThreadId) {
  if (!m_sdlRenderer)
    return;

  if (radius > Dots::RADIUS + 2)
    return;

  if (circleCache.find(radius) == circleCache.end()) {
    CreateCircle(radius);
  }

  // lets just do white
  uint32_t color = (a << 24) | (r << 16) | (g << 8) | b;

  const int rowsPerRegion = Settings::SCREEN_HEIGHT / m_threadPool->num_threads;

  const auto &offsets = circleCache[radius].offsets;
  for (const auto &[dx, dy] : offsets) {
    int absX = centerX + dx;
    int absY = centerY + dy;

    if (absX < 0 || absX >= Settings::SCREEN_WIDTH || absY < 0 ||
        absY >= Settings::SCREEN_HEIGHT)
      continue;

    size_t pixelIndex = absX + absY * Settings::SCREEN_WIDTH;
    size_t regionIndex =
        std::min((int)(m_threadPool->num_threads - 1), absY / rowsPerRegion);

    m_threadSortedPixelData[drawingThreadId][regionIndex].push_back(
        {pixelIndex, color});
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
