# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Rover — a two-board robotics project that drives a physical rover using motors and servos. Repository: `git@github.com:whit3hat/rover.git`, branch `main`.

## Hardware

### Raspberry Pi Zero W (host/server)
- Hosts a web interface for controlling the rover
- Communicates with the Elegoo Mega 2560 (likely over serial/USB)
- WiFi-capable (802.11 b/g/n), single-core 1GHz ARM11, 512MB RAM
- 40-pin GPIO header: 3.3V logic, supports UART, SPI, I2C, PWM
- Docs: https://www.raspberrypi.com/documentation/computers/raspberry-pi.html
- GPIO reference: https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#gpio

### Elegoo Mega 2560 (motor/servo controller)
- Arduino Mega 2560 clone — programmed with the Arduino IDE/toolchain
- ATmega2560 MCU, 16MHz clock, 256KB flash, 8KB SRAM, 4KB EEPROM
- Receives commands from the Pi and controls motors/servos
- 5V logic, 54 digital I/O pins (15 PWM: pins 2–13, 44–46), 16 analog inputs
- 4 hardware UARTs (Serial0–3), SPI, I2C
- USB-B connection to Pi for serial communication and programming
- Docs (PDF): https://m.media-amazon.com/images/I/91RAy+evkrL.pdf
- Motor and servo details TBD

## Architecture (planned)

```
[Browser] <--HTTP/WS--> [Raspberry Pi Zero W: web server] <--serial--> [Elegoo Mega 2560: motor/servo control]
```

- **Pi side**: Web application serving a control UI; sends commands to the Mega over serial
- **Mega side**: Arduino sketch that parses incoming serial commands and drives motors/servos via PWM

## Status

Early stage — no source code, build system, or tests configured yet. Motor/servo specifics to be added as the project progresses.
