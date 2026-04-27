#pragma once

#include <Arduino.h>

namespace Config {

// ============================================================
// THINGS YOU ARE MOST LIKELY TO CHANGE
// ============================================================

namespace Pins {
    constexpr uint8_t oneWireBus = 9;

    constexpr uint8_t heatingPads[] = {11, 10};

    // Each release valve has two synchronized servos.
    // TODO: replace placeholder opposite-side pins with real pin values.
    constexpr uint8_t releaseValveServos[][2] = {
        {5, 30},
        {6, 31},
        {7, 32}
    };

    constexpr uint8_t fanEnable = 29;
    constexpr uint8_t fanPwm = 28;
    constexpr uint8_t sdChipSelect = BUILTIN_SDCARD;
}  // namespace Pins

namespace HeatingPads {
    constexpr size_t count = 2;

    // Heating pad 0 uses temperature sensor 3.
    // Heating pad 1 uses temperature sensor 0.
    constexpr size_t temperatureSensorIndex[count] = {3, 0};

    constexpr float turnOnBelowC = 19.7F;
    constexpr float turnOffAtC = 20.0F;
}  // namespace HeatingPads

namespace ReleaseValves {
    constexpr size_t count = 3;
    constexpr size_t servosPerValve = 2;

    constexpr double openAltitudeMeters[count] = {
        50.0,
        5000.0,
        15000.0
    };

    constexpr int closedAngleDeg = 0;
    constexpr int openAngleDeg = 115;

    constexpr unsigned long cycleDurationMs =
        3UL /*min*/ * 60UL /*sec/min*/ * 1000UL /*ms/sec*/;

    constexpr unsigned long finalValveHoldOpenMs =
        30UL /*min*/ * 60UL /*sec/min*/ * 1000UL /*ms/sec*/;
}  // namespace ReleaseValves

namespace Fan {
    constexpr int offPwm = 0;
    constexpr int onPwm = 128;
}  // namespace Fan

// ============================================================
// TIMING / SAMPLING
// ============================================================

namespace Schedule {
    constexpr unsigned long altitudeUpdatePeriodMs =
        1UL /*sec*/ * 1000UL /*ms/sec*/ / 5UL /*updates/sec*/;

    constexpr unsigned long temperatureUpdatePeriodMs =
        1UL /*sec*/ * 1000UL /*ms/sec*/;

    constexpr unsigned long releaseValveMotionStepPeriodMs =
        1UL /*sec*/ * 1000UL /*ms/sec*/;

    constexpr unsigned long completedMissionIdleDelayMs = 200UL;
}  // namespace Schedule

namespace AltitudeDecision {
    constexpr int samplesRequiredBeforeAction =
        2 /*sec*/ * 5 /*altitude samples/sec*/;
}  // namespace AltitudeDecision

// ============================================================
// SENSOR / MATH SETTINGS
// Usually do not change unless hardware or filtering changes.
// ============================================================

namespace TemperatureSensors {
    constexpr size_t count = 4;

    constexpr float disconnectedC = -127.0F;
    constexpr float minimumValidC = -100.0F;
    constexpr float maximumValidC = 125.0F;
}  // namespace TemperatureSensors

namespace AltitudeMath {
    constexpr double smoothingAlpha = 0.2;
    constexpr double pressureToAltitudeScaleMeters = 44330.0;
    constexpr double pressureToAltitudeExponent = 1.0 / 5.255;
}  // namespace AltitudeMath

namespace BaselinePressure {
    constexpr int sampleCount = 10;
    constexpr unsigned long sampleDelayMs = 20UL;
}  // namespace BaselinePressure

// ============================================================
// HARDWARE PROTOCOL CONSTANTS
// Do not change unless replacing the sensor.
// ============================================================

namespace Ms5607 {
    constexpr uint8_t address = 0x77;
    constexpr size_t calibrationWordCount = 7;

    constexpr uint8_t resetCommand = 0x1E;
    constexpr uint8_t readAdcCommand = 0x00;
    constexpr uint8_t convertPressureCommand = 0x48;
    constexpr uint8_t convertTemperatureCommand = 0x58;
    constexpr uint8_t promBaseCommand = 0xA0;

    constexpr unsigned long resetDelayMs = 10UL;
    constexpr unsigned long conversionDelayMs = 10UL;
}  // namespace Ms5607

// ============================================================
// LOGGING
// ============================================================

namespace Logging {
    constexpr const char* fileName = "log.csv";
    constexpr const char* missingValue = "NA";
}  // namespace Logging

namespace SerialPort {
    constexpr long baudRate = 9600;

    constexpr unsigned long startupWaitMs =
        2UL /*sec*/ * 1000UL /*ms/sec*/;
    }  // namespace SerialPort

}  // namespace Config
