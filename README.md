# Robocar — ATmega328P Line Follower

[![C/C++](https://img.shields.io/badge/C%2FC%2B%2B-bare--metal-00599C.svg?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-IDE-FF7F00.svg?logo=platformio&logoColor=white)](https://platformio.org/)
[![AVR](https://img.shields.io/badge/AVR-ATmega328P-A41E22.svg?logo=arduino&logoColor=white)](https://www.microchip.com/en-us/product/ATmega328P)
[![avr-gcc](https://img.shields.io/badge/avr--gcc-toolchain-555.svg)](https://gcc.gnu.org/wiki/avr-gcc)
[![Bluetooth](https://img.shields.io/badge/HC--05-Bluetooth-0082FC.svg?logo=bluetooth&logoColor=white)](https://en.wikipedia.org/wiki/HC-05)
[![Ultrasonic](https://img.shields.io/badge/HC--SR04-Ultrasonic-607D8B.svg)](https://en.wikipedia.org/wiki/HC-SR04)

A bare-metal C/C++ firmware for an ATmega328P-driven robocar that follows a gradient race track using IR sensors, stops on obstacles via ultrasonic, and accepts remote start/stop over Bluetooth.

---

## What it does

- **Five IR sensors** (front-left, left, middle, right, front-right) sampled with the ADC interrupt to keep the car centered on the gradient track.
- **HC-SR04 ultrasonic** fires every 50 ms from a Timer2 overflow ISR — if anything sits closer than ~17 cm, the car stops.
- **HC-05 Bluetooth** over USART receives single-character commands:
  - `a` → race mode (follow the track)
  - `b` → stop
  - `c` → pause for 5 s, then resume
- **PWM motor control** on Timer0 channels A/B for independent left/right speed.

---

## Stack

- **MCU** — ATmega328P @ 16 MHz
- **Language** — C/C++ with `avr-libc` (`<avr/io.h>`, `<avr/interrupt.h>`, `<util/delay.h>`)
- **Build** — PlatformIO + avr-gcc toolchain
- **Peripherals**
  - 5× IR analog sensors on PC0–PC4 (ADC with auto-channel-switching ISR)
  - HC-SR04 trig PB4 / echo PB5
  - HC-05 Bluetooth on USART0 @ 9600 baud
  - Motor PWM on PD5 / PD6, direction on PD2 / PD4

---

## Pinout

| Function | Port / Pin |
|---|---|
| IR LF / L / M / R / RF | PC0 / PC4 / PC3 / PC2 / PC1 |
| Ultrasonic TRIG | PB4 |
| Ultrasonic ECHO | PB5 |
| Motor PWM Left  | PD5 (OCR0B) |
| Motor PWM Right | PD6 (OCR0A) |
| Motor dir Left  | PD2 |
| Motor dir Right | PD4 |
| Bluetooth RX/TX | PD0 / PD1 (USART0) |

---

## Build & flash

With **PlatformIO**:

```bash
pio run -t upload
pio device monitor -b 9600
```

Or raw **avr-gcc**:

```bash
avr-gcc -mmcu=atmega328p -Os -DF_CPU=16000000UL main.cpp -o main.elf
avr-objcopy -O ihex main.elf main.hex
avrdude -p m328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:main.hex
```

---

## Layout

```
robocar_project/
└── main.cpp     # ISRs (ADC, Timer2, USART RX) + race loop + setup
```

One translation unit — small, fast, easy to follow.

---

© Danielis Maizelis · bare-metal embedded coursework
