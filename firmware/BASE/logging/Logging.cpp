#include "Logging.h"

void Logger::initializeSdCardAndOpenLogFile() {
  if (!SD.begin(Config::Pins::sdChipSelect)) {
    if (Serial) {
      Serial.println("SD card init failed.");
    }
    return;
  }

  file = SD.open(Config::Logging::fileName, FILE_WRITE);

  if (!file) {
    if (Serial) {
      Serial.println("Failed to open log file.");
    }
    return;
  }

  writeHeader();
  file.flush();
}

void Logger::close() {
  if (!file) {
    return;
  }

  file.flush();
  file.close();
}

void Logger::writeRow(
    unsigned long missionStartTimeMs,
    unsigned long currentTimeMs,
    const TemperatureReadings& temperatureReadings,
    const Heating& heating,
    double altitudeMeters,
    const Release& release) {
  if (!file) {
    return;
  }

  file.print(Time::secondsBetween(missionStartTimeMs, currentTimeMs));

  for (size_t sensorIndex = 0;
       sensorIndex < Config::TemperatureSensors::count;
       ++sensorIndex) {
    file.print(",");
    writeTemperature(temperatureReadings.sensorTemperatureC[sensorIndex]);
  }

  file.print(",");
  file.print(temperatureReadings.boardTemperatureC, 1);

  for (size_t heatingPadIndex = 0;
       heatingPadIndex < Config::HeatingPads::count;
       ++heatingPadIndex) {
    file.print(",");
    file.print(heating.heatingPad(heatingPadIndex).isOn ? "ON" : "OFF");
  }

  for (size_t heatingPadIndex = 0;
       heatingPadIndex < Config::HeatingPads::count;
       ++heatingPadIndex) {
    const HeatingPad& pad = heating.heatingPad(heatingPadIndex);

    writeHeatingPadEvent(
        pad.turnedOn,
        pad.lastTurnedOnTemperatureC,
        missionStartTimeMs);
  }

  for (size_t heatingPadIndex = 0;
       heatingPadIndex < Config::HeatingPads::count;
       ++heatingPadIndex) {
    const HeatingPad& pad = heating.heatingPad(heatingPadIndex);

    writeHeatingPadEvent(
        pad.turnedOff,
        pad.lastTurnedOffTemperatureC,
        missionStartTimeMs);
  }

  file.print(",");
  file.print(altitudeMeters, 2);

  for (size_t releaseValveIndex = 0;
       releaseValveIndex < Config::ReleaseValves::count;
       ++releaseValveIndex) {
    file.print(",");
    file.print(
        release.releaseValve(releaseValveIndex).isOpen
            ? "OPEN"
            : "CLOSED");
  }

  file.print(",");
  file.print(release.fan().isOn ? "ON" : "OFF");

  for (size_t releaseValveIndex = 0;
       releaseValveIndex < Config::ReleaseValves::count;
       ++releaseValveIndex) {
    writeSystemEvent(
        release.releaseValve(releaseValveIndex).opened,
        missionStartTimeMs);

    writeSystemEvent(
        release.releaseValve(releaseValveIndex).closed,
        missionStartTimeMs);
  }

  writeSystemEvent(release.fan().turnedOn, missionStartTimeMs);
  writeSystemEvent(release.fan().turnedOff, missionStartTimeMs);

  file.println();
  file.flush();
}

void Logger::writeHeader() {
  file.println(
      "Time_s,"
      "S0_C,S1_C,S2_C,S3_C,Teensy_C,PadA,PadB,"
      "PadA_OnAt_s,PadA_OnTemp_C(S3),"
      "PadB_OnAt_s,PadB_OnTemp_C(S0),"
      "PadA_OffAt_s,PadA_OffTemp_C(S3),"
      "PadB_OffAt_s,PadB_OffTemp_C(S0),"
      "Alt_m,Servo1,Servo2,Servo3,Fan,"
      "S1_OpenAt_s,S1_OpenAlt_m,S1_CloseAt_s,S1_CloseAlt_m,"
      "S2_OpenAt_s,S2_OpenAlt_m,S2_CloseAt_s,S2_CloseAlt_m,"
      "S3_OpenAt_s,S3_OpenAlt_m,S3_CloseAt_s,S3_CloseAlt_m,"
      "Fan_OnAt_s,Fan_OnAlt_m,Fan_OffAt_s,Fan_OffAlt_m");
}

void Logger::writeTemperature(float temperatureC) {
  if (temperatureC == Config::TemperatureSensors::disconnectedC) {
    file.print(Config::Logging::missingValue);
    return;
  }

  file.print(temperatureC, 2);
}

void Logger::writeHeatingPadEvent(
    const LoggedSystemEvent& event,
    float eventTemperatureC,
    unsigned long missionStartTimeMs) {
  file.print(",");

  if (event.happenedDuringCurrentLogRow) {
    file.print(Time::secondsBetween(missionStartTimeMs, event.timeMs));
  } else {
    file.print(Config::Logging::missingValue);
  }

  file.print(",");

  if (event.happenedDuringCurrentLogRow) {
    file.print(eventTemperatureC, 2);
  } else {
    file.print(Config::Logging::missingValue);
  }
}

void Logger::writeSystemEvent(
    const LoggedSystemEvent& event,
    unsigned long missionStartTimeMs) {
  file.print(",");

  if (event.happenedDuringCurrentLogRow) {
    file.print(Time::secondsBetween(missionStartTimeMs, event.timeMs));
  } else {
    file.print(Config::Logging::missingValue);
  }

  file.print(",");

  if (event.happenedDuringCurrentLogRow) {
    file.print(event.altitudeMeters, 2);
  } else {
    file.print(Config::Logging::missingValue);
  }
}
