#include "Release.h"

void Release::initializeHardware() {
  for (size_t releaseValveIndex = 0;
       releaseValveIndex < Config::ReleaseValves::count;
       ++releaseValveIndex) {
    attachReleaseValveServos(releaseValveIndex);
    writeReleaseValvePosition(releaseValveIndex, false);
  }

  pinMode(Config::Pins::fanEnable, OUTPUT);
  pinMode(Config::Pins::fanPwm, OUTPUT);

  analogWrite(Config::Pins::fanPwm, Config::Fan::offPwm);
  digitalWrite(Config::Pins::fanEnable, LOW);
}

void Release::openReleaseValveAndCycleToOpen(
    size_t releaseValveIndex,
    unsigned long currentTimeMs,
    double currentAltitudeMeters) {
  releaseValves[releaseValveIndex].hasCompletedCloseCycle = false;

  writeReleaseValvePosition(releaseValveIndex, true);

  releaseValves[releaseValveIndex].opened.record(
      currentTimeMs,
      currentAltitudeMeters);

  requestReleaseValveCycle(
      releaseValveIndex,
      true,
      false,
      currentTimeMs);
}

void Release::cycleReleaseValveToClosed(
    size_t releaseValveIndex,
    unsigned long currentTimeMs,
    double currentAltitudeMeters) {
  releaseValves[releaseValveIndex].requestedCloseAltitudeMeters =
      currentAltitudeMeters;

  requestReleaseValveCycle(
      releaseValveIndex,
      false,
      true,
      currentTimeMs);
}

void Release::cycleFinalReleaseValveToClosed(
    unsigned long currentTimeMs,
    double currentAltitudeMeters) {
  cycleReleaseValveToClosed(
      finalReleaseValveIndex(),
      currentTimeMs,
      currentAltitudeMeters);
}

void Release::toggleEveryMovingReleaseValveOneStep() {
  for (size_t releaseValveIndex = 0;
       releaseValveIndex < Config::ReleaseValves::count;
       ++releaseValveIndex) {
    if (!releaseValves[releaseValveIndex].cycle.isMoving) {
      continue;
    }

    writeReleaseValvePosition(
        releaseValveIndex,
        !releaseValves[releaseValveIndex].isOpen);
  }
}

void Release::finishReleaseValveCyclesThatReachedDuration(
    unsigned long currentTimeMs,
    unsigned long cycleDurationMs) {
  for (size_t releaseValveIndex = 0;
       releaseValveIndex < Config::ReleaseValves::count;
       ++releaseValveIndex) {
    ReleaseValve& valve = releaseValves[releaseValveIndex];
    ReleaseValveCycle& cycle = valve.cycle;

    if (!cycle.isMoving) {
      continue;
    }

    if (currentTimeMs - cycle.startedAtMs <= cycleDurationMs) {
      continue;
    }

    cycle.isMoving = false;
    writeReleaseValvePosition(releaseValveIndex, cycle.settlesOpen);

    if (!cycle.settlesOpen) {
      valve.hasCompletedCloseCycle = true;
    }

    if (!cycle.settlesOpen && cycle.closeShouldBeLogged) {
      valve.closed.record(
          currentTimeMs,
          valve.requestedCloseAltitudeMeters);

      cycle.closeShouldBeLogged = false;
    }

    if (!cycle.hasQueuedCycle) {
      continue;
    }

    cycle.isMoving = true;
    cycle.settlesOpen = cycle.queuedSettlesOpen;
    cycle.closeShouldBeLogged = cycle.queuedCloseShouldBeLogged;
    cycle.startedAtMs = currentTimeMs;

    cycle.hasQueuedCycle = false;
    cycle.queuedCloseShouldBeLogged = false;
  }
}

void Release::turnFanOn(
    unsigned long currentTimeMs,
    double currentAltitudeMeters) {
  if (fanState.isOn) {
    return;
  }

  digitalWrite(Config::Pins::fanEnable, HIGH);
  analogWrite(Config::Pins::fanPwm, Config::Fan::onPwm);

  fanState.isOn = true;
  fanState.turnedOn.record(currentTimeMs, currentAltitudeMeters);
}

void Release::turnFanOff(
    unsigned long currentTimeMs,
    double currentAltitudeMeters) {
  analogWrite(Config::Pins::fanPwm, Config::Fan::offPwm);
  digitalWrite(Config::Pins::fanEnable, LOW);

  if (!fanState.isOn) {
    return;
  }

  fanState.isOn = false;
  fanState.turnedOff.record(currentTimeMs, currentAltitudeMeters);
}

bool Release::finalReleaseValveIsOpen() const {
  return releaseValves[finalReleaseValveIndex()].isOpen;
}

bool Release::finalReleaseValveIsMoving() const {
  return releaseValves[finalReleaseValveIndex()].cycle.isMoving;
}

bool Release::finalReleaseValveIsIdle() const {
  const ReleaseValveCycle& cycle =
      releaseValves[finalReleaseValveIndex()].cycle;

  return !cycle.isMoving && !cycle.hasQueuedCycle;
}

bool Release::finalReleaseValveHasCompletedCloseCycle() const {
  return releaseValves[finalReleaseValveIndex()].hasCompletedCloseCycle;
}

const ReleaseValve& Release::releaseValve(size_t releaseValveIndex) const {
  return releaseValves[releaseValveIndex];
}

const FanState& Release::fan() const {
  return fanState;
}

void Release::clearLogEvents() {
  for (size_t releaseValveIndex = 0;
       releaseValveIndex < Config::ReleaseValves::count;
       ++releaseValveIndex) {
    releaseValves[releaseValveIndex].opened.clearForNextLogRow();
    releaseValves[releaseValveIndex].closed.clearForNextLogRow();
  }

  fanState.turnedOn.clearForNextLogRow();
  fanState.turnedOff.clearForNextLogRow();
}

void Release::requestReleaseValveCycle(
    size_t releaseValveIndex,
    bool settlesOpen,
    bool closeShouldBeLogged,
    unsigned long currentTimeMs) {
  ReleaseValveCycle& cycle = releaseValves[releaseValveIndex].cycle;

  if (!cycle.isMoving) {
    cycle.isMoving = true;
    cycle.settlesOpen = settlesOpen;
    cycle.closeShouldBeLogged = !settlesOpen && closeShouldBeLogged;
    cycle.startedAtMs = currentTimeMs;
    return;
  }

  if (cycle.settlesOpen == settlesOpen) {
    return;
  }

  cycle.hasQueuedCycle = true;
  cycle.queuedSettlesOpen = settlesOpen;
  cycle.queuedCloseShouldBeLogged = !settlesOpen && closeShouldBeLogged;
}

void Release::attachReleaseValveServos(size_t releaseValveIndex) {
  for (size_t servoIndex = 0;
       servoIndex < Config::ReleaseValves::servosPerValve;
       ++servoIndex) {
    releaseValveServo[releaseValveIndex][servoIndex].attach(
        Config::Pins::releaseValveServos[releaseValveIndex][servoIndex]);
  }
}

void Release::writeReleaseValvePosition(
    size_t releaseValveIndex,
    bool shouldOpen) {
  releaseValves[releaseValveIndex].isOpen = shouldOpen;

  const int targetAngleDeg =
      shouldOpen
          ? Config::ReleaseValves::openAngleDeg
          : Config::ReleaseValves::closedAngleDeg;

  for (size_t servoIndex = 0;
       servoIndex < Config::ReleaseValves::servosPerValve;
       ++servoIndex) {
    releaseValveServo[releaseValveIndex][servoIndex].write(targetAngleDeg);
  }
}

size_t Release::finalReleaseValveIndex() const {
  return Config::ReleaseValves::count - 1;
}
