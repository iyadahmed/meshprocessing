#pragma once

#include <chrono>
#include <iomanip> // for set_precision
#include <iostream>
#include <string>

// Initially-not-but-later insipired-by/largely taken from Blender

using Clock = std::chrono::high_resolution_clock;
using Timepoint = Clock::time_point;
using Nanoseconds = std::chrono::nanoseconds;

void print_duration(Nanoseconds duration) {
  using namespace std::chrono;
  if (duration < microseconds(100)) {
    std::cout << duration.count() << " ns";
  } else if (duration < seconds(5)) {
    std::cout << std::fixed << std::setprecision(1) << duration.count() / 1.0e6
              << " ms";
  } else if (duration > seconds(90)) {
    /* Long durations: print seconds, and also H:m:s */
    const auto dur_hours = duration_cast<hours>(duration);
    const auto dur_mins = duration_cast<minutes>(duration - dur_hours);
    const auto dur_sec =
        duration_cast<seconds>(duration - dur_hours - dur_mins);
    std::cout << std::fixed << std::setprecision(1) << duration.count() / 1.0e9
              << " s (" << dur_hours.count() << "H:" << dur_mins.count()
              << "m:" << dur_sec.count() << "s)";
  } else {
    std::cout << std::fixed << std::setprecision(1) << duration.count() / 1.0e9
              << " s";
  }
}

class [[nodiscard]] ScopedTimer {
private:
  std::string message_;
  Timepoint start_;

public:
  ScopedTimer(std::string message = "Timer") : message_(std::move(message)) {
    start_ = Clock::now();
  }
  ~ScopedTimer() {
    Timepoint end = Clock::now();
    Nanoseconds duration = end - start_;
    std::cout << message_ << " finished in ";
    print_duration(duration);
    std::cout << std::endl;
  }
};

class Timer {
private:
  Timepoint start_;

public:
  Timer() { tick(); }
  void tick() { start_ = Clock::now(); }
  void tock(std::string message = "Timer") {
    Timepoint end = Clock::now();
    Nanoseconds duration = end - start_;
    std::cout << message << " finished in ";
    print_duration(duration);
    std::cout << std::endl;
  }
};
