#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "Config.h"

class Altitude {
 public:
  void initializeSensor();
  void measureBaselinePressure(int sampleCount, unsigned long delayBetweenSamplesMs);
  void updateSmoothedAltitudeFromPressureSensor();

  double currentAltitudeMeters() const;
  double currentPressureHpa() const;
  double currentSensorTemperatureC() const;

 private:
  struct PressureReading {
    double pressureHpa = 0.0;
    double sensorTemperatureC = 0.0;
  };

  void resetPressureSensor();
  void readPressureSensorCalibration();
  uint16_t readCalibrationWord(uint8_t wordIndex);
  uint32_t readAdc(uint8_t conversionCommand);
  PressureReading readPressureSensor();
  double altitudeMetersFromPressure(double pressureHpa) const;

  uint16_t calibrationWord[Config::Ms5607::calibrationWordCount] = {};

  double baselinePressureHpa = 1013.25;
  double pressureHpa = 0.0;
  double sensorTemperatureC = 0.0;
  double smoothedAltitudeMeters = 0.0;

  bool hasAltitudeSample = false;
};
