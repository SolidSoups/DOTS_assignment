#pragma once
#include <chrono>
#include <cstdio>
#include <fstream> // Required for file output
#include <iomanip>
#include <sstream>
#include <string> // Required for std::string
#include <unordered_map>
#include <iostream>

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
  /**
   * @brief Recursively builds the report for a timer and its children.
   * @param ss The stringstream to write the report to.
   * @param name The name of the current timer.
   * @param timer The timer instance to report on.
   * @param level The current indentation level.
   */
  void reportTimerRecursive(std::stringstream &ss, const std::string &name,
                            const Timer &timer, int level) {
    std::stringstream name_ss;
    for (int i = 0; i < level; ++i) {
      name_ss << "    "; // 4 spaces per level
    }
    name_ss << "- " << name << ":";
    std::string indented_name = name_ss.str();

    std::string report = timer.getReport();

    // Write the formatted line to the stringstream
    ss << std::left << std::setw(NAME_COLUMN_WIDTH) << indented_name << report
       << "\n";

    for (const auto &[childName, childTimer] : timer.name_childTimer) {
      reportTimerRecursive(ss, childName, childTimer, level + 1);
    }
  }

public:
  /**
   * @brief Prints the full timer report to the console and optionally saves it
   * to a file.
   * @param saveToFile If true, the report is saved to a text file.
   * The file will be named "SimpleProfiler_report.txt" or
   * "SimpleProfiler_report_{name}.txt" if the profiler has a name.
   */
  void reportTimersFull(bool saveToFile = false) {
    std::stringstream report_ss;
    if (name != "") {
      report_ss << "[SimpleProfiler Timers " << name << " Report]:\n";
    } else {
      report_ss << "[SimpleProfiler Timers Report]:\n";
    }

    for (const auto &[name, timer] : name_timer) {
      reportTimerRecursive(report_ss, name, timer, 0);
    }

    // Get the complete report as a single string
    std::string report_str = report_ss.str();

    // 1. Always print the report to the console
    printf("%s", report_str.c_str());

    // 2. Conditionally save the same report to a file
    if (saveToFile) {
      std::string filename = "SimpleProfiler_report";
      if (!name.empty()) {
        filename += "_" + name;
      }
      filename += ".txt";

      std::ofstream report_file(filename);
      if (report_file.is_open()) {
        report_file << report_str;
        report_file.close();
        printf("\nReport also saved to %s\n", filename.c_str());
      } else {
        printf("\nError: Could not open file %s to save report.\n",
               filename.c_str());
      }
    }
  }
};
