#pragma once
#include <chrono>
#include <unordered_map>
#include <cstdio>

class SimpleProfiler {
private:
  struct Timer {
    std::chrono::high_resolution_clock::time_point start;
    float accumulated = 0;
    int count = 0;
  };

  std::unordered_map<std::string, Timer> timers;

public:
  void start(const std::string& name){
    timers[name].start = std::chrono::high_resolution_clock::now();
  }

  void stop(const std::string& name){
    auto& timer = timers[name];
    auto duration = std::chrono::duration<float, std::milli>(
      std::chrono::high_resolution_clock::now() - timer.start).count();
    timer.accumulated += duration;
    timer.count++;
  }

  void report(){
    printf("[SimpleProfiler Report]:\n");
    for(auto& [name, timer] : timers){
      if(timer.count > 0){
        printf("\t%s: %.2fms avg (%.2fms total, %d calls)\n",
               name.c_str(),
               timer.accumulated / timer.count,
               timer.accumulated,
               timer.count);
        timer.accumulated = 0;
        timer.count = 0;
      }
    }  
  }
};
