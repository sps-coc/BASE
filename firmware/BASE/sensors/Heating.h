#pragma once

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include "Config.h"
#include "Events.h"

struct TemperatureReadings {
  float sensorTemperatureC[Config::TemperatureSensors::count] = {};
  float boardTemperatureC = NAN;
};

struct HeatingPad {
  bool isOn = false;
  bool wasOn = false;

  float lastTurnedOnTemperatureC = NAN;
  float lastTurnedOffTemperatureC = NAN;

  LoggedSystemEvent turnedOn;
  LoggedSystemEvent turnedOff;
};

class Heating {
 public:
  Heating();

  void initializeHardware();

  TemperatureReadings readTemperatureSensors();

  void updateHeatingPadStates(
      const TemperatureReadings& readings,
      unsigned long currentTimeMs,
      double currentAltitudeMeters);

  void writeHeatingPadOutputs() const;
  void forceHeatingPadsOff() const;

  const HeatingPad& heatingPad(size_t heatingPadIndex) const;

  void clearLogEvents();

 private:
  void discoverTemperatureSensors();
  float validTemperatureOrDisconnected(float rawTemperatureC) const;
  bool heatingPadShouldBeOn(float temperatureC, bool currentlyOn) const;

  OneWire oneWire;
  DallasTemperature temperatureSensors;
  DeviceAddress sensorAddress[Config::TemperatureSensors::count];

  HeatingPad heatingPads[Config::HeatingPads::count];
};
