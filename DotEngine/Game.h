#pragma once
#include <vector>
#include <memory>
#include "AABB.h"
#include "Dots.h"
#include "SpatialGrid.h"
#include "SimpleProfiler.h"


class DotRenderer;
class QuadTree;
class ThreadPool;

class Game
{
public:
	Game(DotRenderer* aRenderer, ThreadPool* threadPool, Timer& timer);
  ~Game();
  /*
   * The main update loop for the game. Runs through grid rebuild, updates on dots, collisions and rendering.
   *
   * @param aDeltaTime Deltatime, very useful indeed
   */
	void Update(float aDeltaTime);


  /*
   * Reduction of wild animal population by selective slaughter. May also increases performance.
   * 
   */
  void cullDots(Timer& timer);
  /**
   * Processes collisions for all dots. Runs on the thread pool
   *
   * @param timer Timer for benchmarking
   */
  void processCollisions_threaded(Timer& timer);
  /**
   * Performs collision checks and collision responses. Is thread safe.
   *
   * @param i1 Index of first dot
   * @param i2 Index of other dot
   */
  void collideDots(size_t i1, size_t i2);
  void collideDotsSIMD(size_t i1, size_t i2);
private:
  float timeSinceUpdate;
  /// Owner: Game
  Dots dots;
  std::vector<std::mutex> dots_mutexes;

private:
	DotRenderer* renderer; // self managed
  Timer& timer;
  ThreadPool* threadPool;
  SpatialGrid grid;
};
