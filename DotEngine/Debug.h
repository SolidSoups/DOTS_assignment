#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "SDL3/SDL_pixels.h"

#define DEBUG_MODE_ON

class DotRenderer;
class SDL_Texture;
struct TTF_Font;


struct KeySettings{
  SDL_Color textColor;
};

struct DebugTextInfoItem{
  SDL_Texture* texture;
  int w;
  int h;
};

// TODO: implement a save file for logs? unecessary for now
class Debug {
public:
  Debug(DotRenderer *renderer, TTF_Font* font);
  static Debug* GetInstance();
  static void DeleteInstance();

  ~Debug();
  void Render();

private:
  static Debug* Instance;
  DotRenderer *_renderer;
  TTF_Font* m_font;

public: // screen debug
  static void UpdateScreenField(std::string key, std::string value);
  static void UpdateKeySettings(std::string key, KeySettings settings);
private:
  std::unordered_map<std::string, std::string> debugValuesMap;
  std::unordered_map<std::string, KeySettings> keySettingsMap;
  std::unordered_map<std::string, DebugTextInfoItem> textDebugInfoMap;
  std::vector<std::string> keysOrder;

public: // console logging functions
  static void Log(const std::string &msg);
  static void LogWarning(const std::string &msg);
  static void LogError(const std::string &msg);
};
