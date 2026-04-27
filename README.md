# BASE

BASE is the flight-control system for a balloon payload. It runs on a Teensy microcontroller and manages sensing (altitude, temperature), actuation (release valves, heating pads, fan), and SD-card logging during the mission.

---

# Quick Start (TL;DR)

If you just want to build and upload the firmware:

1. Install **VS Code**
2. Install the **PlatformIO extension**
3. Clone this repo
4. Plug in the Teensy
5. Open the repo in VS Code
6. Click **Upload** in PlatformIO

That’s it. See details below if anything fails.

---

# Repository Overview

```text
BASE/
  platformio.ini        ← build configuration (PlatformIO)
  
  firmware/             ← ALL microcontroller code
    BASE/
      main.cpp          ← entry point (runs on Teensy)
      Config.h          ← all configuration

      mission/          ← mission logic / state machine
      sensors/          ← altitude + temperature sensing
      control/          ← heating, release valves, fan
      logging/          ← SD card logging
      common/           ← shared utilities

  hardware/             ← CAD, wiring, schematics, calculations
  docs/                 ← procedures, guides, mission details
  data/                 ← sample logs and outputs
```

---

# Firmware Setup (PlatformIO + Teensy)

## 1. Install tools

- Install **VS Code**
- Install **PlatformIO extension** inside VS Code

## 2. Clone the repository

```bash
git clone https://github.com/sps-coc/BASE.git
cd BASE
```

## 3. Connect hardware

- Plug in the Teensy via USB
- Ensure it powers on

## 4. Open project

- Open the `BASE/` folder in VS Code
- PlatformIO will automatically detect `platformio.ini`

## 5. Build

Click:

```text
PlatformIO sidebar → Project Tasks → teensy41 → Build
```

or run:

```bash
pio run
```

## 6. Upload

Click:

```text
PlatformIO sidebar → Project Tasks → teensy41 → Upload
```

or run:

```bash
pio run --target upload
```

## 7. Serial monitor (optional)

```bash
pio device monitor
```

---

# Important: Board Configuration

The project is configured for:

```ini
board = teensy41
```

If you are using a different Teensy, change it in `platformio.ini`.

Examples:

```ini
teensy40
teensy36
teensy35
```

---

# System Architecture

The firmware is organized by subsystem:

- **Mission** → central state machine controlling flight behavior  
- **Sensors**
  - Altitude (pressure sensor)
  - Temperature sensors  
- **Control**
  - Heating pads
  - Release valves (servos)
  - Fan  
- **Logging**
  - Writes CSV data to SD card  
- **Common**
  - Timing + event utilities  

---

# Mission Sequence

1. Initialize hardware (sensors, servos, SD card)
2. Record baseline pressure
3. Begin logging to SD card
4. Continuously estimate altitude
5. Trigger release valves at configured thresholds
6. Control heating pads based on temperature
7. Run fan when required
8. Complete mission and safely shut down

---

# Configuration

All mission parameters are in:

```text
firmware/BASE/Config.h
```

This includes:

- Pin assignments
- Altitude thresholds
- Temperature thresholds
- Servo angles
- Timing intervals
- Logging settings

**You MUST verify these before flight.**

---

# Logging

The system writes:

```text
log.csv
```

to the SD card.

Includes:

- Time
- Altitude
- Temperature readings
- Heating states/events
- Release valve states/events
- Fan state

---

# Testing Checklist (DO THIS BEFORE FLIGHT)

## Basic checks

- [ ] Board powers on
- [ ] Firmware uploads successfully
- [ ] Serial output is visible

## Sensors

- [ ] Pressure readings change with altitude
- [ ] Temperature sensors return valid values

## Actuators

- [ ] Servos move correctly (no payload attached)
- [ ] Heating pads switch on/off correctly
- [ ] Fan turns on/off

## Logging

- [ ] SD card initializes
- [ ] `log.csv` is created
- [ ] Data is written correctly

---

# Hardware

See:

```text
hardware/
```

Contains:

- CAD models (STL, STEP, SolidWorks)
- Wiring diagrams
- Schematics
- Engineering calculations
- Design justifications

---

# Documentation

See:

```text
docs/ (in progress)
```

For:

- Assembly instructions
- Mission details
- Testing procedures
- Troubleshooting

---

# For New Contributors

Start here:

1. Read `docs/mission-overview.md`
2. Read `firmware/BASE/Config.h`
3. Read `firmware/BASE/mission/Mission.cpp`
4. Build and upload firmware
5. Run a dry test

---

# ❓ Troubleshooting

### Upload fails

- Press the Teensy reset button and try again
- Ensure correct board is selected

### Build errors

- Run:
  ```bash
  pio run -t clean
  ```
- Rebuild

### Sensors not working

- Check wiring
- Check pin assignments in `Config.h`

---

# Notes

- This is a **mission-critical system** — always test before flight
- Never attach payload when testing actuators
- Validate configuration every time before deployment
