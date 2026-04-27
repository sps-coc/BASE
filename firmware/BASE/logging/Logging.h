#pragma once

#include <Arduino.h>
#include <SD.h>

#include "Altitude.h"
#include "Config.h"
#include "Heating.h"
#include "Release.h"
#include "Time.h"

class Logger {
 public:
  void initializeSdCardAndOpenLogFile();
  void close();

  void writeRow(
      unsigned long missionStartTimeMs,
      unsigned long currentTimeMs,
      const TemperatureReadings& temperatureReadings,
      const Heating& heating,
      double altitudeMeters,
      const Release& release);

 private:
  void writeHeader();

  void writeTemperature(float temperatureC);
  void writeHeatingPadEvent(
      const LoggedSystemEvent& event,
      float eventTemperatureC,
      unsigned long missionStartTimeMs);

  void writeSystemEvent(
      const LoggedSystemEvent& event,
      unsigned long missionStartTimeMs);

  File file;
};