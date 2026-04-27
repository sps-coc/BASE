#pragma once

#include <Arduino.h>
#include <Servo.h>

#include "Config.h"
#include "Events.h"

struct ReleaseValveCycle {
  bool isMoving = false;
  bool settlesOpen = true;
  bool closeShouldBeLogged = false;
  unsigned long startedAtMs = 0;

  bool hasQueuedCycle = false;
  bool queuedSettlesOpen = true;
  bool queuedCloseShouldBeLogged = false;
};

struct ReleaseValve {
  bool isOpen = false;
  bool hasCompletedCloseCycle = false;

  double requestedCloseAltitudeMeters = 0.0;

  ReleaseValveCycle cycle;

  LoggedSystemEvent opened;
  LoggedSystemEvent closed;
};

struct FanState {
  bool isOn = false;

  LoggedSystemEvent turnedOn;
  LoggedSystemEvent turnedOff;
};

class Release {
 public:
  void initializeHardware();

  void openReleaseValveAndCycleToOpen(
      size_t releaseValveIndex,
      unsigned long currentTimeMs,
      double currentAltitudeMeters);

  void cycleReleaseValveToClosed(
      size_t releaseValveIndex,
      unsigned long currentTimeMs,
      double currentAltitudeMeters);

  void cycleFinalReleaseValveToClosed(
      unsigned long currentTimeMs,
      double currentAltitudeMeters);

  void toggleEveryMovingReleaseValveOneStep();

  void finishReleaseValveCyclesThatReachedDuration(
      unsigned long currentTimeMs,
      unsigned long cycleDurationMs);

  void turnFanOn(unsigned long currentTimeMs, double currentAltitudeMeters);
  void turnFanOff(unsigned long currentTimeMs, double currentAltitudeMeters);

  bool finalReleaseValveIsOpen() const;
  bool finalReleaseValveIsMoving() const;
  bool finalReleaseValveIsIdle() const;
  bool finalReleaseValveHasCompletedCloseCycle() const;

  const ReleaseValve& releaseValve(size_t releaseValveIndex) const;
  const FanState& fan() const;

  void clearLogEvents();

 private:
  void requestReleaseValveCycle(
      size_t releaseValveIndex,
      bool settlesOpen,
      bool closeShouldBeLogged,
      unsigned long currentTimeMs);

  void attachReleaseValveServos(size_t releaseValveIndex);
  void writeReleaseValvePosition(size_t releaseValveIndex, bool shouldOpen);

  size_t finalReleaseValveIndex() const;

  Servo releaseValveServo[Config::ReleaseValves::count][Config::ReleaseValves::servosPerValve];
  ReleaseValve releaseValves[Config::ReleaseValves::count];
  FanState fanState;
};
