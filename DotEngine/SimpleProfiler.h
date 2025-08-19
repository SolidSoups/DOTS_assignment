#pragma once
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

struct Timer {
  static constexpr int MAX_LEVELS = 5;

private:
  std::chrono::high_resolution_clock::time_point start;

public:
  float accumulated = 0;
  int count = 0;

  int level = 0;
  std::unordered_map<std::string, Timer> name_childTimer;

  void startClock() { start = std::chrono::high_resolution_clock::now(); }

  void stopClock() {
    auto duration = std::chrono::duration<float, std::milli>(
                        std::chrono::high_resolution_clock::now() - start)
                        .count();
    accumulated += duration;
    count++;
  }

  Timer &startChild(const std::string &name) {
    auto &timer = name_childTimer[name];
    timer.startClock();
    timer.level = level + 1;
    return timer;
  };

  const std::string getSimpleReport(const std::string &prependStr) const {
    std::stringstream ss;
    if (count > 0) {
      ss << prependStr << ": " << std::fixed << std::setprecision(2)
         << accumulated / count << "ms avg";
    } else {
      ss << "n/a";
    }
    return ss.str();
  }

  const std::string getReport() const {
    std::stringstream ss;
    if (count > 0) {
      ss << std::fixed << std::setprecision(2) << accumulated / count
         << "ms avg (" << accumulated << "ms total, " << count << " calls)";
    } else {
      ss << "Insufficient data...";
    }
    return ss.str();
  }
};

class SimpleProfiler {
private:
  static constexpr int NAME_COLUMN_WIDTH = 40;
  // new
  std::unordered_map<std::string, Timer> name_timer;
  std::string name{""};

public:
  SimpleProfiler() = default;
  SimpleProfiler(std::string name) : name(name) {}
  SimpleProfiler(const SimpleProfiler &other) = delete;

  /// Gets/Creates and starts a timer
  Timer &start(const std::string &name) {
    auto &timer = name_timer[name];
    timer.startClock();
    return timer;
  }

private:
  void reportTimerRecursive(const std::string &name, const Timer &timer,
                            int level) {
    std::stringstream name_ss;
    for (int i = 0; i < level; ++i) {
      name_ss << "    "; // 4 spaces per level
    }
    name_ss << "- " << name << ":";
    std::string indented_name = name_ss.str();

    std::string report = timer.getReport();

    printf("%-*s%s\n", NAME_COLUMN_WIDTH, indented_name.c_str(),
           report.c_str());

    for (const auto &[childName, childTimer] : timer.name_childTimer) {
      reportTimerRecursive(childName, childTimer, level + 1);
    }
  }

public:
  void reportTimersFull() {
    if(name != ""){
      printf("[SimpleProfiler Timers %s Report]:\n", name.c_str());
    } else{
      printf("[SimpleProfiler Timers Report]:\n");
    }
    for (const auto &[name, timer] : name_timer) {
      reportTimerRecursive(name, timer, 0);
    }
  }
};
