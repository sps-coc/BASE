#include "Altitude.h"
#include "Config.h"

void Altitude::initializeSensor() {
  Wire.begin();
  resetPressureSensor();
  readPressureSensorCalibration();
}

void Altitude::measureBaselinePressure(
    int sampleCount,
    unsigned long delayBetweenSamplesMs) {
  double pressureSumHpa = 0.0;

  for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
    pressureSumHpa += readPressureSensor().pressureHpa;
    delay(delayBetweenSamplesMs);
  }

  baselinePressureHpa = pressureSumHpa / sampleCount;
  hasAltitudeSample = false;
}

void Altitude::updateSmoothedAltitudeFromPressureSensor() {
  const PressureReading reading = readPressureSensor();

  pressureHpa = reading.pressureHpa;
  sensorTemperatureC = reading.sensorTemperatureC;

  const double rawAltitudeMeters = altitudeMetersFromPressure(pressureHpa);

  if (!hasAltitudeSample) {
    smoothedAltitudeMeters = rawAltitudeMeters;
    hasAltitudeSample = true;
    return;
  }

  smoothedAltitudeMeters =
      Config::AltitudeMath::smoothingAlpha * rawAltitudeMeters +
      (1.0 - Config::AltitudeMath::smoothingAlpha) * smoothedAltitudeMeters;
}

double Altitude::currentAltitudeMeters() const {
  return smoothedAltitudeMeters;
}

double Altitude::currentPressureHpa() const {
  return pressureHpa;
}

double Altitude::currentSensorTemperatureC() const {
  return sensorTemperatureC;
}

void Altitude::resetPressureSensor() {
  Wire.beginTransmission(Config::Ms5607::address);
  Wire.write(Config::Ms5607::resetCommand);
  Wire.endTransmission();
  delay(Config::Ms5607::resetDelayMs);
}

void Altitude::readPressureSensorCalibration() {
  for (size_t wordIndex = 0;
       wordIndex < Config::Ms5607::calibrationWordCount;
       ++wordIndex) {
    calibrationWord[wordIndex] = readCalibrationWord(wordIndex);
  }
}

uint16_t Altitude::readCalibrationWord(uint8_t wordIndex) {
  Wire.beginTransmission(Config::Ms5607::address);
  Wire.write(Config::Ms5607::promBaseCommand + wordIndex * 2);
  Wire.endTransmission();

  Wire.requestFrom(Config::Ms5607::address, static_cast<uint8_t>(2));

  return (static_cast<uint16_t>(Wire.read()) << 8) | Wire.read();
}

uint32_t Altitude::readAdc(uint8_t conversionCommand) {
  Wire.beginTransmission(Config::Ms5607::address);
  Wire.write(conversionCommand);
  Wire.endTransmission();

  delay(Config::Ms5607::conversionDelayMs);

  Wire.beginTransmission(Config::Ms5607::address);
  Wire.write(Config::Ms5607::readAdcCommand);
  Wire.endTransmission();

  Wire.requestFrom(Config::Ms5607::address, static_cast<uint8_t>(3));

  return (static_cast<uint32_t>(Wire.read()) << 16) |
         (static_cast<uint32_t>(Wire.read()) << 8) |
         static_cast<uint32_t>(Wire.read());
}

Altitude::PressureReading Altitude::readPressureSensor() {
  const uint32_t rawPressure =
      readAdc(Config::Ms5607::convertPressureCommand);

  const uint32_t rawTemperature =
      readAdc(Config::Ms5607::convertTemperatureCommand);

  const int32_t temperatureDifference =
      static_cast<int32_t>(rawTemperature) -
      (static_cast<int32_t>(calibrationWord[5]) << 8);

  const int32_t temperatureCentiC =
      2000 +
      (static_cast<int64_t>(temperatureDifference) * calibrationWord[6]) /
          (1 << 23);

  const int64_t offset =
      (static_cast<int64_t>(calibrationWord[2]) << 16) +
      (static_cast<int64_t>(calibrationWord[4]) * temperatureDifference) /
          (1 << 7);

  const int64_t sensitivity =
      (static_cast<int64_t>(calibrationWord[1]) << 15) +
      (static_cast<int64_t>(calibrationWord[3]) * temperatureDifference) /
          (1 << 8);

  const int32_t pressureCentiHpa = static_cast<int32_t>(
      (((static_cast<int64_t>(rawPressure) * sensitivity) / (1 << 21)) -
       offset) /
      (1 << 15));

  PressureReading reading;
  reading.pressureHpa = pressureCentiHpa / 100.0;
  reading.sensorTemperatureC = temperatureCentiC / 100.0;
  return reading;
}

double Altitude::altitudeMetersFromPressure(double pressureHpaValue) const {
  return Config::AltitudeMath::pressureToAltitudeScaleMeters *
         (1.0 - pow(
                    pressureHpaValue / baselinePressureHpa,
                    Config::AltitudeMath::pressureToAltitudeExponent));
}
