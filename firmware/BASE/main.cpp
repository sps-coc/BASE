#include <Arduino.h>
#include "Mission.h"

Mission balloonMission;

void setup() {
  balloonMission.initializeHardware();
  balloonMission.startMission();
}

void loop() {
  if (balloonMission.hardwareHasBeenStopped()) {
    balloonMission.enterSafeIdleAfterHardwareStop();
    return;
  }

  const unsigned long currentTimeMs = millis();

  balloonMission.updateAltitudeControl(currentTimeMs);
  balloonMission.updateReleaseValveMotion(currentTimeMs);
  balloonMission.updateTemperatureControlAndLogging(currentTimeMs);
}
