#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>
#include <cstdint>

class DotRenderer {
private:
  struct CirclePixels{
    std::vector<std::pair<int, int>> offsets;
  };
  std::unordered_map<int, CirclePixels> circleCache;
  void CreateCircle(int radius);
  uint32_t* pixelBuffer = nullptr;
  SDL_Texture* frameTexture;

  Uint8 red;
  Uint8 green;
  Uint8 blue;
  Uint8 alpha;
  
public:
  DotRenderer(SDL_Window *window);

  ~DotRenderer();

  SDL_Renderer *GetSDLRenderer() const { return m_sdlRenderer; }


  void SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
  void Clear();
  void Present();

  void DrawRect(float mx, float my, float Mx, float My);
  void DrawFilledCircle(int centerX, int centerY, int radius, int r, int g, int b, int a);
  void RenderTexture(SDL_Texture *texture, const SDL_FRect *srcRect,
                     const SDL_FRect *dstRect);

private:
  SDL_Renderer *m_sdlRenderer;

  void DrawPoint(int x, int y);

  DotRenderer(const DotRenderer &) = delete;
  DotRenderer &operator=(const DotRenderer &) = delete;
};
