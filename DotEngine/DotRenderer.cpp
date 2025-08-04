#include "DotRenderer.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL.h>
#include <cmath> 

DotRenderer::DotRenderer(SDL_Window* window) : m_sdlRenderer(nullptr)
{
	m_sdlRenderer = SDL_CreateRenderer(window, nullptr);
	if (!m_sdlRenderer) return;
}

DotRenderer::~DotRenderer()
{
	if (m_sdlRenderer)
	{
		SDL_DestroyRenderer(m_sdlRenderer);
		m_sdlRenderer = nullptr;
	}
}

void DotRenderer::SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if (m_sdlRenderer)
	{
		SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, a);
	}
}

void DotRenderer::Clear()
{
	if (m_sdlRenderer)
	{
		SDL_RenderClear(m_sdlRenderer);
	}
}

void DotRenderer::Present()
{
	if (m_sdlRenderer)
	{
		SDL_RenderPresent(m_sdlRenderer);
	}
}

void DotRenderer::DrawPoint(int x, int y)
{
	if (m_sdlRenderer)
	{
		SDL_RenderPoint(m_sdlRenderer, x, y);
	}
}

void DotRenderer::DrawCircle(int centerX, int centerY, int radius)
{
	if (!m_sdlRenderer) return;

	int x = radius;
	int y = 0;
	int err = 0;

	while (x >= y) 
	{
		DrawPoint(centerX + x, centerY + y);
		DrawPoint(centerX + y, centerY + x);
		DrawPoint(centerX - y, centerY + x);
		DrawPoint(centerX - x, centerY + y);
		DrawPoint(centerX - x, centerY - y);
		DrawPoint(centerX - y, centerY - x);
		DrawPoint(centerX + y, centerY - x);
		DrawPoint(centerX + x, centerY - y);

		if (err <= 0) 
		{
			y++;
			err += 2 * y + 1;
		}
		if (err > 0) 
		{
			x--;
			err -= 2 * x + 1;
		}
	}
}

void DotRenderer::DrawFilledCircle(int centerX, int centerY, int radius)
{
	if (!m_sdlRenderer) return;

	for (int y = -radius; y <= radius; y++) 
	{
		int x = static_cast<int>(std::sqrt(radius * radius - y * y));
		SDL_RenderLine(m_sdlRenderer, centerX - x, centerY + y, centerX + x, centerY + y);
	}
}

void DotRenderer::RenderTexture(SDL_Texture* texture, const SDL_FRect* srcRect, const SDL_FRect* dstRect)
{
	if (m_sdlRenderer && texture)
	{
		SDL_RenderTexture(m_sdlRenderer, texture, srcRect, dstRect);
	}
}




