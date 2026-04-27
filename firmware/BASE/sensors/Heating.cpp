#include "Heating.h"

Heating::Heating()
    : oneWire(Config::Pins::oneWireBus),
      temperatureSensors(&oneWire) {}

void Heating::initializeHardware() {
  for (size_t heatingPadIndex = 0;
       heatingPadIndex < Config::HeatingPads::count;
       ++heatingPadIndex) {
    pinMode(Config::Pins::heatingPads[heatingPadIndex], OUTPUT);
    digitalWrite(Config::Pins::heatingPads[heatingPadIndex], LOW);
  }

  temperatureSensors.begin();
  discoverTemperatureSensors();
}

TemperatureReadings Heating::readTemperatureSensors() {
  TemperatureReadings readings;

  temperatureSensors.requestTemperatures();

  for (size_t sensorIndex = 0;
       sensorIndex < Config::TemperatureSensors::count;
       ++sensorIndex) {
    readings.sensorTemperatureC[sensorIndex] =
        validTemperatureOrDisconnected(
            temperatureSensors.getTempC(sensorAddress[sensorIndex]));
  }

  readings.boardTemperatureC = tempmonGetTemp();

  return readings;
}

void Heating::updateHeatingPadStates(
    const TemperatureReadings& readings,
    unsigned long currentTimeMs,
    double currentAltitudeMeters) {
  for (size_t heatingPadIndex = 0;
       heatingPadIndex < Config::HeatingPads::count;
       ++heatingPadIndex) {
    HeatingPad& pad = heatingPads[heatingPadIndex];

    const size_t sensorIndex =
        Config::HeatingPads::temperatureSensorIndex[heatingPadIndex];

    const float padTemperatureC =
        readings.sensorTemperatureC[sensorIndex];

    pad.wasOn = pad.isOn;
    pad.isOn = heatingPadShouldBeOn(padTemperatureC, pad.isOn);

    const bool justTurnedOn = !pad.wasOn && pad.isOn;
    const bool justTurnedOff = pad.wasOn && !pad.isOn;

    if (justTurnedOn) {
      pad.lastTurnedOnTemperatureC = padTemperatureC;
      pad.turnedOn.record(currentTimeMs, currentAltitudeMeters);
    }

    if (justTurnedOff) {
      pad.lastTurnedOffTemperatureC = padTemperatureC;
      pad.turnedOff.record(currentTimeMs, currentAltitudeMeters);
    }
  }
}

void Heating::writeHeatingPadOutputs() const {
  for (size_t heatingPadIndex = 0;
       heatingPadIndex < Config::HeatingPads::count;
       ++heatingPadIndex) {
    digitalWrite(
        Config::Pins::heatingPads[heatingPadIndex],
        heatingPads[heatingPadIndex].isOn ? HIGH : LOW);
  }
}

void Heating::forceHeatingPadsOff() const {
  for (size_t heatingPadIndex = 0;
       heatingPadIndex < Config::HeatingPads::count;
       ++heatingPadIndex) {
    digitalWrite(Config::Pins::heatingPads[heatingPadIndex], LOW);
  }
}

const HeatingPad& Heating::heatingPad(size_t heatingPadIndex) const {
  return heatingPads[heatingPadIndex];
}

void Heating::clearLogEvents() {
  for (size_t heatingPadIndex = 0;
       heatingPadIndex < Config::HeatingPads::count;
       ++heatingPadIndex) {
    heatingPads[heatingPadIndex].turnedOn.clearForNextLogRow();
    heatingPads[heatingPadIndex].turnedOff.clearForNextLogRow();
  }
}

void Heating::discoverTemperatureSensors() {
  for (size_t sensorIndex = 0;
       sensorIndex < Config::TemperatureSensors::count;
       ++sensorIndex) {
    if (!oneWire.search(sensorAddress[sensorIndex])) {
      break;
    }
  }

  oneWire.reset_search();
}

float Heating::validTemperatureOrDisconnected(float rawTemperatureC) const {
  const bool disconnected =
      rawTemperatureC == Config::TemperatureSensors::disconnectedC;

  const bool outsideValidRange =
      rawTemperatureC < Config::TemperatureSensors::minimumValidC ||
      rawTemperatureC > Config::TemperatureSensors::maximumValidC;

  if (disconnected || outsideValidRange) {
    return Config::TemperatureSensors::disconnectedC;
  }

  return rawTemperatureC;
}

bool Heating::heatingPadShouldBeOn(
    float temperatureC,
    bool currentlyOn) const {
  if (temperatureC == Config::TemperatureSensors::disconnectedC) {
    return false;
  }

  if (currentlyOn) {
    return temperatureC < Config::HeatingPads::turnOffAtC;
  }

  return temperatureC < Config::HeatingPads::turnOnBelowC;
}
