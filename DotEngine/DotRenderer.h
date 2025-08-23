#pragma once
#include "SimpleProfiler.h"
#include <SDL3/SDL.h>
#include <atomic>
#include <cstdint>
#include <unordered_map>
#include <vector>

class ThreadPool;
struct Timer;

class DotRenderer {
private:
  bool isOutOfBounds(int x, int y) const;

private:
  // struct CirclePixels {
  //   std::vector<std::pair<int, int>> offsets;
  // };

  struct CircleSpan {
    int y_offset;
    int x_start_offset;
    int length;
  };

  struct CirclePixels{
    std::vector<CircleSpan> spans;
  };

  std::unordered_map<int, CirclePixels> circleCache;
  void CreateCircle(int radius);

  uint32_t *m_combinedPixelBuffer;
  const size_t bufferSize;

  SDL_Texture *frameTexture;

  ThreadPool *m_threadPool;
  Timer &timer;

public:
  DotRenderer(SDL_Window *window, ThreadPool *threadPool, Timer &timer);

  ~DotRenderer();

  SDL_Renderer *GetSDLRenderer() const { return m_sdlRenderer; }

  void SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  void Clear();
  void Present();

  void DrawRect(float mx, float my, float Mx, float My);
  void RenderTexture(SDL_Texture *texture, const SDL_FRect *srcRect,
                     const SDL_FRect *dstRect);
  void BatchDrawCirclesCPUThreaded(float pos_x[], float pos_y[],
                                   uint8_t radii[],
                                   std::vector<size_t> aliveIndices,
                                   Timer& timer);
  /*
  * Blends the pixels of the src and dst register, and outputs the result to the dst buffer. Uses SIMD to process 4 pixels at a time.
  *
  * @param src The src pixel color buffer
  * @param dst The dst pixel color buffer
  * @param size The size of the buffers
  */
  void BlendAdditiveSIMD(uint32_t *src, uint32_t *dst, size_t size);
  void BlendSolidColorSIMD(uint32_t color, uint32_t* dst_buffer, size_t size);
  /*
  * Blends the src and dst pixels and returns the result
  *
  * @param src The src pixel color
  * @param dst The dst pixel color
  * @return The blended color value of that pixel
  */
  uint32_t BlendAdditive(uint32_t src, uint32_t dst);

private:
  SDL_Renderer *m_sdlRenderer;

  void DrawPoint(int x, int y);

  DotRenderer(const DotRenderer &) = delete;
  DotRenderer &operator=(const DotRenderer &) = delete;
};
