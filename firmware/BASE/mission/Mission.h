#pragma once

#include <Arduino.h>

#include "Altitude.h"
#include "Config.h"
#include "Heating.h"
#include "Logging.h"
#include "Release.h"
#include "Time.h"

class Mission {
 public:
  void initializeHardware();
  void startMission();

  void updateAltitudeControl(unsigned long currentTimeMs);
  void updateReleaseValveMotion(unsigned long currentTimeMs);
  void updateTemperatureControlAndLogging(unsigned long currentTimeMs);

  bool hardwareHasBeenStopped() const;
  void enterSafeIdleAfterHardwareStop();

 private:
  void advanceReleaseValveSequenceFromAltitude(unsigned long currentTimeMs);

  void openNextReleaseValveIfAltitudeIsStableEnough(
      unsigned long currentTimeMs);

  void requestFinalReleaseValveCloseIfHoldTimeHasElapsed(
      unsigned long currentTimeMs);

  void finishMissionAfterFinalReleaseValveCloses(
      unsigned long currentTimeMs);

  void incrementOrResetAltitudeSampleCounts();

  bool releaseValveAltitudeHasBeenReachedForEnoughSamples(
      size_t releaseValveIndex) const;

  bool finalReleaseValveIsReadyToClose(unsigned long currentTimeMs) const;
  bool finalReleaseValveHasFinishedClosing() const;

  void stopAllHardwareAndCloseLog();

  Altitude altitude;
  Heating heating;
  Release release;
  Logger logger;

  Time::PeriodicTask altitudeTask;
  Time::PeriodicTask temperatureTask;
  Time::PeriodicTask releaseValveMotionTask;

  int altitudeSampleCountByReleaseValve[Config::ReleaseValves::count] = {};

  size_t nextReleaseValveToOpen = 0;

  unsigned long missionStartTimeMs = 0;
  unsigned long finalReleaseValveFinishedOpeningAtMs = 0;

  bool finalReleaseValveCloseHasBeenRequested = false;
  bool missionComplete = false;
  bool hardwareStopped = false;
};