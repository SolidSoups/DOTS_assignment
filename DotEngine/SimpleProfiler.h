#pragma once
#include <chrono>
#include <unordered_map>
#include <cstdio>
#include <sstream>
#include <cmath>

class SimpleProfiler {
private:
  struct Timer {
    std::chrono::high_resolution_clock::time_point start;
    float accumulated = 0;
    int count = 0;
  };
  struct Counter {
    int value = 0;
    int accumulated = 0;
    int count = 0;
  };

  std::unordered_map<std::string, Timer> timers;
  std::unordered_map<std::string, Counter> counters;

public: // Timers
  void startTimer(const std::string& name){
    timers[name].start = std::chrono::high_resolution_clock::now();
  }

  void stopTimer(const std::string& name){
    auto& timer = timers[name];
    auto duration = std::chrono::duration<float, std::milli>(
      std::chrono::high_resolution_clock::now() - timer.start).count();
    timer.accumulated += duration;
    timer.count++;
  }

  void resetTimer(const std::string& name){
    auto& timer = timers[name];
    timer.accumulated = 0;
    timer.count = 0;
  }

  void resetTimers(){
    for(auto& [_, timer] : timers){
      timer.count = 0;
      timer.accumulated = 0;
    }
  }

public: // Counters
  void addCounter(const std::string& name, int value=1){
    auto& counter = counters[name]; 
    counter.value += value;
  }
  void stopCounter(const std::string& name){
    auto& counter = counters[name];
    counter.accumulated += counter.value;
    counter.value = 0;
    counter.count++;
  }

  void resetCounter(const std::string& name){
    auto& counter = counters[name];
    counter.value = 0;
    counter.accumulated = 0;
    counter.count = 0;
  }

  void resetCounters(){
    for(auto& [_, counter] : counters){
      counter.value = 0;
      counter.accumulated = 0;
      counter.count = 0;
    }
  }

public: 
  const std::string getFormattedTimer(const std::string& name){
    auto& timer = timers[name];
    std::stringstream ss;
    ss << name << ": ";
    if(timer.count > 0){
      float avg = timer.accumulated / timer.count;
      float rounded = std::round(avg * 100.0f) / 100.0f;
      ss << rounded << "ms avg";
      timer.accumulated = 0;
      timer.count = 0;
    } else{
      ss << "n/a";
    }

    return ss.str();
  }

  const std::string getFormattedCounter(const std::string& name){
    auto& counter = counters[name];
    std::stringstream ss;
    ss << name << ": ";
    if(counter.count > 0){
      ss << counter.accumulated / float(counter.count) << " avg";
    } else{
      ss << "n/a";
    }
    return ss.str();
  }

public: // output reports
  void reportCounter(const std::string& name){
    auto& counter = counters[name];
    if(counter.count > 0){
      printf("[SimpleProfiler Counters Report]:\n");
      printf("\t%s: %.2fms avg (%d total, %d calls)\n",
             name.c_str(),
             counter.accumulated / float(counter.count),
             counter.accumulated,
             counter.count);
    } else{
      printf("\t%s: Insufficient data...\n", name.c_str());
    }
  }
  void reportCountersFull(){
    printf("[SimpleProfiler Counters Report]:\n");
    for(auto& [name, counter] : counters){
      if(counter.count > 0){
        printf("\t%s: %.2fms avg (%d total, %d calls)\n",
               name.c_str(),
               counter.accumulated / float(counter.count),
               counter.accumulated,
               counter.count);
      } else{
        printf("\t%s: Insufficient data...\n", name.c_str());
      }
    }
  }

  void reportTimer(const std::string& name){
    printf("[SimpleProfiler Timers Report]:\n");
    auto& timer = timers[name];
    if(timer.count > 0){
      printf("\t%s: %.2fms avg (%.2fms total, %d calls)\n",
             name.c_str(),
             timer.accumulated / timer.count,
             timer.accumulated,
             timer.count);
    } else{
      printf("\t%s: Insufficient data...\n", name.c_str());
    }
  }

  void reportTimersFull(){
    printf("[SimpleProfiler Timers Report]:\n");
    for(auto& [name, timer] : timers){
      if(timer.count > 0){
        printf("\t%s: %.2fms avg (%.2fms total, %d calls)\n",
               name.c_str(),
               timer.accumulated / timer.count,
               timer.accumulated,
               timer.count);
      } else{
        printf("\t%s: Insufficient data...\n", name.c_str());
      }
    }  
  }
};
