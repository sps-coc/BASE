#pragma once

#include <Arduino.h>

struct LoggedSystemEvent {
  bool happenedDuringCurrentLogRow = false;
  unsigned long timeMs = 0;
  double altitudeMeters = 0.0;

  void record(unsigned long eventTimeMs, double eventAltitudeMeters) {
    happenedDuringCurrentLogRow = true;
    timeMs = eventTimeMs;
    altitudeMeters = eventAltitudeMeters;
  }

  void clearForNextLogRow() {
    happenedDuringCurrentLogRow = false;
  }
};