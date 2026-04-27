#pragma once

#include <Arduino.h>

namespace Time {

struct PeriodicTask {
  unsigned long lastRunTimeMs = 0;

  bool isDue(unsigned long currentTimeMs, unsigned long periodMs) {
    if (currentTimeMs - lastRunTimeMs < periodMs) {
      return false;
    }

    lastRunTimeMs = currentTimeMs;
    return true;
  }
};

inline long secondsBetween(unsigned long startTimeMs, unsigned long endTimeMs) {
  return static_cast<long>((endTimeMs - startTimeMs) / 1000UL);
}

}  // namespace Time