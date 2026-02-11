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

## Architecture

```
[Browser] <--HTTP/WS--> [Raspberry Pi Zero W: web server] <--serial--> [Elegoo Mega 2560: motor/servo control]
```

- **Pi side**: Web application serving a control UI; sends commands to the Mega over serial
- **Mega side**: Arduino sketch that parses incoming serial commands and drives motors/servos via PWM

## Project Structure

```
rover/
├── server/
│   ├── main.py            # FastAPI app, WebSocket endpoint, static file serving
│   ├── serial_comm.py     # Async serial connection manager (pyserial-asyncio)
│   └── requirements.txt   # fastapi, uvicorn[standard], pyserial-asyncio
├── frontend/
│   ├── index.html         # Control dashboard UI
│   ├── style.css          # Dark theme, mobile-friendly styling
│   └── app.js             # WebSocket client, keyboard/touch input handling
├── CLAUDE.md
└── README.md
```

## Stack

- **Backend**: Python 3 + FastAPI + uvicorn (async, native WebSocket support)
- **Serial**: pyserial-asyncio for non-blocking Pi → Mega communication
- **Frontend**: Plain HTML/CSS/JS — no build step, served as static files by FastAPI
- **Python version**: 3.9 on dev machine (avoid 3.10+ syntax like `str | None`)

## Running the Server

```bash
cd server
pip3 install -r requirements.txt
python3 -m uvicorn main:app --host 0.0.0.0 --port 8000
```

Opens at `http://localhost:8000`. When no serial device is present at `/dev/ttyUSB0`, the server runs in **dev mode** — commands are logged to console instead of sent over serial.

## Serial Command Protocol

Text-based commands sent from Pi to Mega over serial (`/dev/ttyUSB0`, 9600 baud, newline-terminated):

| Command | Action   |
|---------|----------|
| `FWD`   | Forward  |
| `BCK`   | Backward |
| `LFT`   | Left     |
| `RGT`   | Right    |
| `STP`   | Stop     |

Commands are validated server-side in `main.py` (`VALID_COMMANDS` set). The frontend sends commands on key/button press and auto-sends `STP` on release for continuous movement control.

## Key Implementation Details

- **WebSocket endpoint**: `/ws` — clients send `{"cmd": "FWD"}`, receive `{"type": "ack", "cmd": "FWD"}` or `{"type": "error", "msg": "..."}`
- **Connection status**: On WS connect, server sends `{"type": "status", "serial": true/false}` so the UI can show Serial vs Dev Mode badge
- **Frontend controls**: Arrow keys, WASD, and on-screen touch buttons. All mapped in `KEY_MAP` in `app.js`
- **Auto-reconnect**: Frontend reconnects on WebSocket disconnect (1s interval)
- **Serial fallback**: `SerialConnection` in `serial_comm.py` gracefully handles missing serial device and missing pyserial-asyncio import

## Next Steps

- Arduino sketch for the Elegoo Mega 2560 (parse serial commands, drive motors/servos)
- Define motor/servo pin assignments and wiring
- Extend command protocol (e.g., speed control, servo angles)
- Deploy and test on actual Pi Zero W hardware
