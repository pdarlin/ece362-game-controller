
# ECE 362 Handheld Game Console

## Project Description

An RP2350-based handheld gaming console developed for Purdue University's ECE 362 Microprocessor Systems and Interfacing course.

The console features a 2.8-inch SPI TFT LCD, microSD card storage, motion controls, dual analog joysticks, a D-pad, action buttons, PWM haptic feedback, and audio output through a passive buzzer.

## Project Objectives

### 1. Game Display (SPI)
- Interface with a 2.8-inch ILI9341 TFT LCD (240 × 320).
- Display game graphics, menus, and user interfaces.
- Implement SPI communication for screen updates.

### 2. Game Storage (SPI)
- Interface with a microSD card for external storage.
- Implement file reading for game assets and data.

### 3. Motion Control (I2C)
- Interface with the MPU motion sensor using I2C.
- Read accelerometer and gyroscope data.
- Implement motion-based game controls.

### 4. Audio and Haptic Feedback (PWM)
- Generate sound effects using a passive buzzer.
- Control vibration motors using PWM.
- Implement adjustable vibration intensity for game events.

## Additional Features

- Dual analog joysticks
- D-pad and tactile action buttons
- Custom PCB designed using KiCad
- Modular firmware using the Raspberry Pi Pico SDK

## Firmware Structure

```text
firmware/
├── include/
│   ├── audio.h
│   ├── buttons.h
│   ├── display.h
│   ├── game.h
│   ├── gyro.h
│   ├── haptic.h
│   ├── joystick.h
│   └── sdcard.h
├── src/
│   ├── audio.c
│   ├── buttons.c
│   ├── display.c
│   ├── game.c
│   ├── gyro.c
│   ├── haptic.c
│   ├── joystick.c
│   ├── main.c
│   └── sdcard.c
└── platformio.ini
```

*Some firmware modules are still under development.*

## Development Tools

- **Microcontroller:** Raspberry Pi RP2350
- **Programming Language:** C
- **Framework:** Raspberry Pi Pico SDK
- **Build System:** PlatformIO
- **PCB Design:** KiCad

## Team Members

- Pierson Darling
- Tejas Menmow
- Tristan Paul
- Alex Chen

## Project Status

In development — Fall 2026.
