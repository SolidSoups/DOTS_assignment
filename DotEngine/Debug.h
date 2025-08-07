#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#define DEBUG_MODE_ON

class DotRenderer;
class SDL_Texture;
struct TTF_Font;

struct DebugTextInfoItem{
  SDL_Texture* texture;
  int w;
  int h;
};

// TODO: implement a save file for logs? unecessary for now
class Debug {
public:
  static Debug* Instance;
  static Debug& GetInstance(DotRenderer *renderer, TTF_Font* font);
  static void DeleteInstance();

  ~Debug();
  void Render();

private:
  Debug(DotRenderer *renderer, TTF_Font* font);
  DotRenderer *_renderer;
  TTF_Font* m_font;

public: // screen debug
  void UpdateScreenField(std::string key, std::string value);
private:
  std::unordered_map<std::string, std::string> debugValuesMap;
  std::unordered_map<std::string, DebugTextInfoItem> textDebugInfoMap;
  std::vector<std::string> keysOrder;

public: // console logging functions
  static void Log(const std::string &msg);
  static void LogWarning(const std::string &msg);
  static void LogError(const std::string &msg);
};
