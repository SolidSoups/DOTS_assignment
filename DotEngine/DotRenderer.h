#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <atomic>

class ThreadPool;

class DotRenderer {
private:
  static constexpr int MAX_THREADS = 8;

private:
  struct CirclePixels{
    std::vector<std::pair<int, int>> offsets;
  };
  struct Pixel{
    size_t index;
    uint32_t color;
  };
  // Sorted like so: [drawingThreadId][regionIndex][pixel]
  //
  // Each drawing thread sorts the pixels into regions, which 
  // each combining thread then can iterate through without 
  // needing to sort the pixels beforehand.
  std::vector<std::vector<std::vector<Pixel>>> m_threadSortedPixelData;

  std::unordered_map<int, CirclePixels> circleCache;
  void CreateCircle(int radius);

  uint32_t* m_combinedPixelBuffer;
  const size_t bufferSize;

  SDL_Texture* frameTexture;

  ThreadPool* m_threadPool;

  Uint8 red;
  Uint8 green;
  Uint8 blue;
  Uint8 alpha;
  
public:
  DotRenderer(SDL_Window *window, ThreadPool* threadPool);

  ~DotRenderer();

  SDL_Renderer *GetSDLRenderer() const { return m_sdlRenderer; }


  void SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  void Clear();
  void Present();

  void DrawRect(float mx, float my, float Mx, float My);
  void RenderTexture(SDL_Texture *texture, const SDL_FRect *srcRect,
                     const SDL_FRect *dstRect);
  void DrawFilledCircle(int centerX,
                        int centerY,
                        int radius,
                        int r,
                        int g,
                        int b,
                        int a,
                        int drawingThreadId);
  void BatchDrawCirclesCPUThreaded(
    float pos_x[],
    float pos_y[],
    uint8_t radii[],
    int red[],
    size_t size
  );
  void CombineThreadBuffers(
    const std::vector<std::vector<std::vector<Pixel>>> &pixelData,
    uint32_t* outputBuffer
  );
  uint32_t BlendAdditive(uint32_t src, uint32_t dst);

private:
  SDL_Renderer *m_sdlRenderer;

  void DrawPoint(int x, int y);

  DotRenderer(const DotRenderer &) = delete;
  DotRenderer &operator=(const DotRenderer &) = delete;
};
