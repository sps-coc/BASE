#include "Config.h"
#include "Mission.h"

void Mission::initializeHardware() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(9600);

  const unsigned long serialWaitStartedAtMs = millis();
  while (!Serial &&
         millis() - serialWaitStartedAtMs < Config::SerialPort::startupWaitMs) {
  }

  heating.initializeHardware();
  release.initializeHardware();
  logger.initializeSdCardAndOpenLogFile();
  altitude.initializeSensor();
}

void Mission::startMission() {
  altitude.measureBaselinePressure(
      Config::BaselinePressure::sampleCount,
      Config::BaselinePressure::sampleDelayMs);

  missionStartTimeMs = millis();
  digitalWrite(LED_BUILTIN, HIGH);
}

void Mission::updateAltitudeControl(unsigned long currentTimeMs) {
  if (hardwareStopped) {
    return;
  }

  if (!altitudeTask.isDue(
          currentTimeMs,
          Config::Schedule::altitudeUpdatePeriodMs)) {
    return;
  }

  altitude.updateSmoothedAltitudeFromPressureSensor();
  incrementOrResetAltitudeSampleCounts();
  advanceReleaseValveSequenceFromAltitude(currentTimeMs);
}

void Mission::updateReleaseValveMotion(unsigned long currentTimeMs) {
  if (hardwareStopped) {
    return;
  }

  if (releaseValveMotionTask.isDue(
          currentTimeMs,
          Config::Schedule::releaseValveMotionStepPeriodMs)) {
    release.toggleEveryMovingReleaseValveOneStep();
  }

  const bool finalReleaseValveWasStillOpening =
      release.finalReleaseValveIsOpen() &&
      release.finalReleaseValveIsMoving() &&
      finalReleaseValveFinishedOpeningAtMs == 0;

  release.finishReleaseValveCyclesThatReachedDuration(
      currentTimeMs,
      Config::ReleaseValves::cycleDurationMs);

  const bool finalReleaseValveJustFinishedOpening =
      finalReleaseValveWasStillOpening &&
      release.finalReleaseValveIsOpen() &&
      release.finalReleaseValveIsIdle() &&
      finalReleaseValveFinishedOpeningAtMs == 0;

  if (finalReleaseValveJustFinishedOpening) {
    finalReleaseValveFinishedOpeningAtMs = currentTimeMs;
  }
}

void Mission::updateTemperatureControlAndLogging(
    unsigned long currentTimeMs) {
  if (hardwareStopped) {
    return;
  }

  if (!temperatureTask.isDue(
          currentTimeMs,
          Config::Schedule::temperatureUpdatePeriodMs)) {
    return;
  }

  const TemperatureReadings readings =
      heating.readTemperatureSensors();

  heating.updateHeatingPadStates(
      readings,
      currentTimeMs,
      altitude.currentAltitudeMeters());

  heating.writeHeatingPadOutputs();

  logger.writeRow(
      missionStartTimeMs,
      currentTimeMs,
      readings,
      heating,
      altitude.currentAltitudeMeters(),
      release);

  heating.clearLogEvents();
  release.clearLogEvents();

  if (missionComplete) {
    stopAllHardwareAndCloseLog();
  }
}

bool Mission::hardwareHasBeenStopped() const {
  return hardwareStopped;
}

void Mission::enterSafeIdleAfterHardwareStop() {
  heating.forceHeatingPadsOff();
  delay(Config::Schedule::completedMissionIdleDelayMs);
}

void Mission::advanceReleaseValveSequenceFromAltitude(
    unsigned long currentTimeMs) {
  openNextReleaseValveIfAltitudeIsStableEnough(currentTimeMs);
  requestFinalReleaseValveCloseIfHoldTimeHasElapsed(currentTimeMs);
  finishMissionAfterFinalReleaseValveCloses(currentTimeMs);
}

void Mission::openNextReleaseValveIfAltitudeIsStableEnough(
    unsigned long currentTimeMs) {
  if (nextReleaseValveToOpen >= Config::ReleaseValves::count) {
    return;
  }

  if (!releaseValveAltitudeHasBeenReachedForEnoughSamples(
          nextReleaseValveToOpen)) {
    return;
  }

  const double currentAltitudeMeters =
      altitude.currentAltitudeMeters();

  release.openReleaseValveAndCycleToOpen(
      nextReleaseValveToOpen,
      currentTimeMs,
      currentAltitudeMeters);

  const bool previousReleaseValveExists =
      nextReleaseValveToOpen > 0;

  if (previousReleaseValveExists) {
    release.cycleReleaseValveToClosed(
        nextReleaseValveToOpen - 1,
        currentTimeMs,
        currentAltitudeMeters);
  }

  const bool openedFinalReleaseValve =
      nextReleaseValveToOpen == Config::ReleaseValves::count - 1;

  if (openedFinalReleaseValve) {
    release.turnFanOn(currentTimeMs, currentAltitudeMeters);
    finalReleaseValveFinishedOpeningAtMs = 0;
    finalReleaseValveCloseHasBeenRequested = false;
  }

  altitudeSampleCountByReleaseValve[nextReleaseValveToOpen] = 0;
  ++nextReleaseValveToOpen;
}

void Mission::requestFinalReleaseValveCloseIfHoldTimeHasElapsed(
    unsigned long currentTimeMs) {
  if (!finalReleaseValveIsReadyToClose(currentTimeMs)) {
    return;
  }

  release.cycleFinalReleaseValveToClosed(
      currentTimeMs,
      altitude.currentAltitudeMeters());

  finalReleaseValveCloseHasBeenRequested = true;
}

void Mission::finishMissionAfterFinalReleaseValveCloses(
    unsigned long currentTimeMs) {
  if (!finalReleaseValveHasFinishedClosing()) {
    return;
  }

  release.turnFanOff(
      currentTimeMs,
      altitude.currentAltitudeMeters());

  missionComplete = true;
}

void Mission::incrementOrResetAltitudeSampleCounts() {
  for (size_t releaseValveIndex = 0;
       releaseValveIndex < Config::ReleaseValves::count;
       ++releaseValveIndex) {
    const bool altitudeHasReachedReleaseValveThreshold =
        altitude.currentAltitudeMeters() >=
        Config::ReleaseValves::openAltitudeMeters[releaseValveIndex];

    altitudeSampleCountByReleaseValve[releaseValveIndex] =
        altitudeHasReachedReleaseValveThreshold
            ? min(
                  altitudeSampleCountByReleaseValve[releaseValveIndex] + 1,
                  Config::AltitudeDecision::samplesRequiredBeforeAction)
            : 0;
  }
}

bool Mission::releaseValveAltitudeHasBeenReachedForEnoughSamples(
    size_t releaseValveIndex) const {
  return altitudeSampleCountByReleaseValve[releaseValveIndex] >=
        Config::AltitudeDecision::altitudeSamplesRequiredBeforeAction;
}

bool Mission::finalReleaseValveIsReadyToClose(
    unsigned long currentTimeMs) const {
  if (nextReleaseValveToOpen < Config::ReleaseValves::count) {
    return false;
  }

  if (finalReleaseValveFinishedOpeningAtMs == 0) {
    return false;
  }

  if (finalReleaseValveCloseHasBeenRequested) {
    return false;
  }

  if (!release.finalReleaseValveIsIdle()) {
    return false;
  }

  return currentTimeMs - finalReleaseValveFinishedOpeningAtMs >=
         Config::ReleaseValves::finalValveHoldOpenMs;
}

bool Mission::finalReleaseValveHasFinishedClosing() const {
  return finalReleaseValveCloseHasBeenRequested &&
         release.finalReleaseValveIsIdle() &&
         release.finalReleaseValveHasCompletedCloseCycle();
}

void Mission::stopAllHardwareAndCloseLog() {
  if (hardwareStopped) {
    return;
  }

  heating.forceHeatingPadsOff();
  release.turnFanOff(millis(), altitude.currentAltitudeMeters());
  logger.close();

  digitalWrite(LED_BUILTIN, LOW);
  hardwareStopped = true;
}
