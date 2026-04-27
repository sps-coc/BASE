# BASE

BASE is the flight-control software for a balloon payload system. It runs on an Arduino/Teensy-style board and controls altitude-triggered release valves, heating pads, fan output, pressure sensing, temperature sensing, and SD-card logging.

## What this system does

During flight, BASE:
1. Initializes sensors, servos, heating pads, fan, serial output, and SD logging.
2. Measures baseline pressure before launch.
3. Continuously estimates altitude from the MS5607 pressure sensor.
4. Opens release valves at configured altitude thresholds.
5. Controls heating pads using temperature thresholds.
6. Logs mission data to `log.csv`.
7. Stops hardware safely after the mission completes.

## Repository structure

| File | Purpose |
|---|---|
| `BASE.ino` | Arduino entry point. Calls mission setup and loop updates. |
| `Mission.h/.cpp` | Main mission coordinator and state machine. |
| `Config.h` | Hardware pins, thresholds, timing, sensor constants, logging settings. |
| `Altitude.h/.cpp` | MS5607 pressure sensor interface and altitude calculation. |
| `Heating.h/.cpp` | Temperature sensors and heating pad control. |
| `Release.h/.cpp` | Servo release valve and fan control. |
| `Logging.h/.cpp` | SD-card CSV logging. |
| `Events.h` | Shared event-recording helper. |
| `Time.h` | Timing helper for periodic tasks. |

## Hardware assumptions

- Pressure sensor: MS5607 over I2C
- Temperature sensors: Dallas/OneWire sensors
- Release valves: 3 valves, 2 servos per valve
- Heating pads: 2 pads
- Fan: PWM + enable pin
- Storage: SD card using `BUILTIN_SDCARD`

## Configuration

Most mission-specific values are in `Config.h`.

Important settings:
- `Config::Pins`: board pin assignments
- `Config::HeatingPads`: heating thresholds and sensor mapping
- `Config::ReleaseValves`: altitude thresholds, servo angles, cycle timing
- `Config::Schedule`: update rates
- `Config::Logging`: CSV filename and missing-value marker

Before flight, verify:
- Release valve servo pins
- Heating pad pins
- Temperature sensor ordering
- Altitude thresholds
- Servo open/closed angles
- SD card availability

## Mission sequence

1. Start hardware.
2. Record baseline pressure.
3. Begin logging.
4. Sample altitude at the configured rate.
5. Open each release valve after its altitude threshold is reached for enough samples.
6. Keep the final valve open for the configured hold time.
7. Close the final valve.
8. Turn off hardware and enter safe idle.

## Log output

The system writes `log.csv` to the SD card.

The log includes:
- Mission time
- Four external temperature sensor readings
- Teensy/internal board temperature
- Heating pad states and heating events
- Altitude
- Release valve states and release events
- Fan state and fan events

## Dependencies

Install these Arduino libraries:
- Servo
- Wire
- SD
- OneWire
- DallasTemperature

Also use a board/platform that supports:
- `BUILTIN_SDCARD`
- `tempmonGetTemp()`

## Build and upload

1. Open `BASE.ino` in the Arduino IDE.
2. Select the correct board and port.
3. Install the required libraries.
4. Verify/compile.
5. Upload to the flight controller.

## Safety checklist

Before running the mission:
- Test servos without payload attached.
- Confirm valve open/closed angles.
- Confirm heater output pins with a multimeter.
- Confirm temperature sensor ordering.
- Confirm SD logging creates `log.csv`.
- Confirm pressure sensor readings are reasonable.
- Run a dry test using safe altitude thresholds.

## Known TODOs

- Replace placeholder release-valve servo pins in `Config.h`.
- Document wiring in `docs/hardware.md`.
- Add example `log.csv`.
- Add a mission-state diagram.
