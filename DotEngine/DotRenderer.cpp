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
#include <immintrin.h>
#include <iostream>
#include <thread>

DotRenderer::DotRenderer(SDL_Window *window, ThreadPool *threadPool,
                         Timer &timer)
    : m_sdlRenderer(nullptr), m_threadPool(threadPool), timer(timer),
      bufferSize(Settings::SCREEN_WIDTH * Settings::SCREEN_HEIGHT) {
  m_sdlRenderer = SDL_CreateRenderer(window, nullptr);
  if (!m_sdlRenderer)
    return;

  // The intermediate buffer m_threadSortedPixelData is no longer needed and has
  // been removed.
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
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      if ((x * x + y * y) <= radius * radius) { // inside the circle
        int startX = x;
        while ((x * x) + (y * y) <= radius * radius) {
          x++;
        }
        newCircle.spans.push_back({y, startX, x - startX});
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

// =========================================================================================
// SINGLE-PASS IMPLEMENTATION
// =========================================================================================
void DotRenderer::BatchDrawCirclesCPUThreaded(
    float pos_x[], float pos_y[], uint8_t radii[],
    std::vector<size_t> aliveIndices, Timer& timer) {
  Timer& t_total = timer.startChild("batch_rendering");
  size_t size = aliveIndices.size();

  if (!m_sdlRenderer || size == 0)
    return;


  // Clear the buffer for the new frame.
  memset(m_combinedPixelBuffer, 0, bufferSize * sizeof(uint32_t));

  const int nThreads = m_threadPool->num_threads;
  const int rowsPerThread = Settings::SCREEN_HEIGHT / nThreads;

  // RENDER THREADING: One job per screen-space region
  auto &t_drawing = t_total.startChild("drawing_and_blending");
  auto &t_queueing = t_drawing.startChild("queuing_jobs");
  for (int t = 0; t < nThreads; ++t) {
    // 1. Divide the screen into horizontal regions (rows) for each thread
    const int startY = t * rowsPerThread;
    const int endY =
        (t == nThreads - 1) ? Settings::SCREEN_HEIGHT : startY + rowsPerThread;

    m_threadPool->queueJob(
      [this, &aliveIndices, pos_x, pos_y, radii, size, startY, endY]() {
        for (size_t di = 0; di < size; ++di) {
          size_t index = aliveIndices[di];

          const int cX = static_cast<int>(pos_x[index]);
          const int cY = static_cast<int>(pos_y[index]);
          const int radius = radii[index];

          // quick bound check if to skip dots not in this region
          if(cY + radius < startY || cY - radius >= endY)
            continue;

          // find the cached dot
          auto it = circleCache.find(radius);
          if(it == circleCache.end())
            continue;

          // calculate color of this dot
          constexpr float foo = 0.5f * 255.f * 4.f;
          uint8_t red = (radii[index] - Dots::RADIUS) * foo;
          // ARGB
          uint32_t color = (255 << 24) | (red << 16) | (125 << 8) | 125; 

          for(const auto &span : it->second.spans){
            int pixelY = cY + span.y_offset;

            // draw pixel span ONLY if it falls within this threads region
            if(pixelY >= startY && pixelY < endY){
              int startX = cX + span.x_start_offset;
              int endX = startX + span.length;

              int clampedStartX = std::max(0, startX);
              int clampedEndX = std::min(Settings::SCREEN_WIDTH, endX);
              int clampedLength = clampedEndX - clampedStartX;

              if(clampedLength > 0){
                size_t pixelIndex = clampedStartX + pixelY * Settings::SCREEN_WIDTH;

                // blend the entire contiguos scanline using the SIMD function
                BlendSolidColorSIMD(color, m_combinedPixelBuffer + pixelIndex, clampedLength);
              }
            }
          }
        }
      });
  }
  t_queueing.stopClock();

  // wait only ONCE for all rendering jobs to complete.
  auto &t_wait = t_drawing.startChild("wait_for_threads");
  m_threadPool->wait();
  t_wait.stopClock();
  t_drawing.stopClock();

  // update and render texture
  auto &t_sdlCalls = t_total.startChild("sdl_calls");
  SDL_UpdateTexture(frameTexture, nullptr, m_combinedPixelBuffer,
                    Settings::SCREEN_WIDTH * sizeof(uint32_t));
  SDL_RenderTexture(m_sdlRenderer, frameTexture, nullptr, nullptr);
  t_sdlCalls.stopClock();
  t_total.stopClock();
}

void DotRenderer::BlendSolidColorSIMD(uint32_t color, uint32_t *dst_buffer,
                                      size_t size) {
  // Create a 128-bit register with the solid color broadcast to all 4 lanes
  __m128i src_pixels = _mm_set1_epi32(color);

  // unpack the src pixels and interleave them with zeros (for additive
  // saturation)
  __m128i zero = _mm_setzero_si128();
  __m128i src_lo = _mm_unpacklo_epi8(src_pixels, zero);
  __m128i src_hi = _mm_unpackhi_epi8(src_pixels, zero);

  size_t i = 0;
  for (; i + 3 < size; i += 4) {
    // load the dst pixels into a 128-bit (4x4 byte integers)
    __m128i dst_pixels = _mm_loadu_si128((__m128i *)(dst_buffer + i));

    // Unpack the 8-bit dst pixels into an interleaved 16-bit format (2x4 byte
    // integers, each spaced with 2 byte zero's)
    __m128i dst_lo = _mm_unpacklo_epi8(dst_pixels, zero);
    // add the result of both the src and dst lows, the 's' in 'adds' ensures
    // saturation if the value overflows 16-bits (ie 65535), then it is set to
    // the max
    __m128i result_lo = _mm_adds_epu16(src_lo, dst_lo);

    // Unpack the 8-bit dst pixels into an interleaved 16-bit format (2x4 bytes
    // integers, each spaced with 2 zeros)
    __m128i dst_hi = _mm_unpackhi_epi8(dst_pixels, zero);
    // add the result of the hi's with saturation
    __m128i result_hi = _mm_adds_epu16(src_hi, dst_hi);

    // combine the interleaved hi and lo into one register, adding each lane
    // together
    __m128i result = _mm_packus_epi16(result_lo, result_hi);
    _mm_storeu_si128((__m128i *)(dst_buffer + i), result);
  }

  for (; i < size; ++i) {
    dst_buffer[i] = BlendAdditive(color, dst_buffer[i]);
  }
}

void DotRenderer::BlendAdditiveSIMD(uint32_t *src_buffer, uint32_t *dst_buffer,
                                    size_t size) {
  // Process 4 pixels at a time
  size_t i = 0;
  for (; i + 3 < size; i += 4) {
    // Load 4 pixels from each buffer into 128-bit registers
    __m128i src_pixels = _mm_loadu_si128((__m128i *)(src_buffer + i));
    __m128i dst_pixels = _mm_loadu_si128((__m128i *)(dst_buffer + i));

    // used for the interleaving, allowing us to add two "unpacked"
    // registers together with the overflow going in the "zero spots"
    // which can later be ignored
    __m128i zero = _mm_setzero_si128();

    // Unpack the 8-bit channels into 16-bit
    __m128i src_lo = _mm_unpacklo_epi8(src_pixels, zero);
    __m128i dst_lo = _mm_unpacklo_epi8(dst_pixels, zero);
    // Add the channels with "unsigned saturation", automatically
    // clamping the result to 0-255
    __m128i result_lo = _mm_adds_epu16(src_lo, dst_lo);

    // Repack the 16-bit words back into 8-bit channels
    // We do this for the high half as well
    __m128i src_hi = _mm_unpackhi_epi8(src_pixels, zero);
    __m128i dst_hi = _mm_unpackhi_epi8(dst_pixels, zero);
    __m128i result_hi = _mm_adds_epu16(src_hi, dst_hi);

    __m128i result = _mm_packus_epi16(result_lo, result_hi);

    // store the 4 blended pixels back into the destination buffer
    _mm_storeu_si128((__m128i *)(dst_buffer + i), result);
  }

  // Handle any leftover pixels (if size is not a multiple of 4)
  for (; i < size; ++i) {
    dst_buffer[i] = BlendAdditive(src_buffer[i], dst_buffer[i]);
  }
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

bool DotRenderer::isOutOfBounds(int x, int y) const {
  return x < 0 || x >= Settings::SCREEN_WIDTH || y < 0 ||
         y >= Settings::SCREEN_HEIGHT;
}

// DrawFilledCircle, CombineThreadBuffers, and getRegionIndex have been removed.

void DotRenderer::DrawRect(float minX, float minY, float maxX, float maxY) {
  if (!m_sdlRenderer)
    return;

  SDL_FRect dst{
    minX, minY,
    maxX, maxY
  };
  SDL_RenderFillRect(m_sdlRenderer, &dst);

  // SDL_RenderLine(m_sdlRenderer, minX, minY, maxX, minY); // top
  // SDL_RenderLine(m_sdlRenderer, maxX, minY, maxX, maxY); // right
  // SDL_RenderLine(m_sdlRenderer, minX, maxY, maxX, maxY); // bottom
  // SDL_RenderLine(m_sdlRenderer, minX, minY, minX, maxY); // top
}

void DotRenderer::RenderTexture(SDL_Texture *texture, const SDL_FRect *srcRect,
                                const SDL_FRect *dstRect) {
  if (m_sdlRenderer && texture) {
    SDL_RenderTexture(m_sdlRenderer, texture, srcRect, dstRect);
  }
}
