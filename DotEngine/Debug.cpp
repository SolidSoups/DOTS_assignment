#include "Debug.h"
#include "DotRenderer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// std
#include <iostream>

// initialization
Debug::Debug(DotRenderer *renderer, TTF_Font *font)
    : _renderer(renderer), m_font(font) {}

Debug::~Debug(){
  for(const auto& item_kvp : textDebugInfoMap){
    SDL_DestroyTexture(item_kvp.second.texture);
  } 
}

// screen logging
void Debug::Render() {
  const float TextSpacing = 10.f;
  float incrementalHeight = 0.f;
  for(int i=0; i<keysOrder.size(); ++i){
    auto& item = textDebugInfoMap[keysOrder[i]];
    SDL_FRect renderQuad = {0, incrementalHeight, (float)item.w,
      (float)item.h};
    _renderer->RenderTexture(item.texture, nullptr, &renderQuad);
    incrementalHeight += (float)item.h + TextSpacing;
  }
}

void Debug::UpdateScreenField(std::string key, std::string value) {
  auto [iterator, bInserted] = debugValuesMap.insert({key, value});

  if (!bInserted){
    // the key already exists
    if(iterator->second != value)
      // the value has changed, update it
      iterator->second = value;
    else{
      // the value is the same, 
      // no need to generate a new texture
      return;
    }
  }
  else{
    // it's a new item, keep track of order
    keysOrder.push_back(key);
  }

  // update the texture
  SDL_Surface *textSurface =
      TTF_RenderText_Solid(m_font, value.c_str(), 0, {255, 255, 255, 255});
  if (textSurface != nullptr) {
    SDL_Texture *textTexture =
        SDL_CreateTextureFromSurface(_renderer->GetSDLRenderer(), textSurface);
    // update text info map
    textDebugInfoMap[key] = {textTexture, textSurface->w, textSurface->h};
  }
  // TODO: delete surface
  SDL_DestroySurface(textSurface);

  Log("There are currently " + std::to_string(textDebugInfoMap.size()) + " elements in the map.");
}

// console logging

void Debug::Log(const std::string &msg) {
#ifdef DEBUG_MODE_ON
  std::cout << "[LOG] " << msg << "\n";
#endif
}

void Debug::LogWarning(const std::string &msg) {
#ifdef DEBUG_MODE_ON
  std::cout << "[WARNING] " << msg << "\n";
#endif
}

void Debug::LogError(const std::string &msg) {
#ifdef DEBUG_MODE_ON
  std::cout << "[ERROR] " << msg << "\n";
#endif
}
