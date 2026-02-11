import logging
from contextlib import asynccontextmanager
from pathlib import Path

from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles

from serial_comm import SerialConnection

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

VALID_COMMANDS = {"FWD", "BCK", "LFT", "RGT", "STP"}
FRONTEND_DIR = Path(__file__).resolve().parent.parent / "frontend"

serial = SerialConnection()


@asynccontextmanager
async def lifespan(app: FastAPI):
    await serial.connect()
    logger.info("Rover server started")
    yield
    await serial.disconnect()
    logger.info("Rover server stopped")


app = FastAPI(lifespan=lifespan)

# Track connected WebSocket clients
clients: set[WebSocket] = set()


@app.websocket("/ws")
async def websocket_endpoint(ws: WebSocket):
    await ws.accept()
    clients.add(ws)
    logger.info(f"Client connected ({len(clients)} total)")
    try:
        await ws.send_json({"type": "status", "serial": serial.connected})
        while True:
            data = await ws.receive_json()
            cmd = data.get("cmd", "").upper()
            if cmd in VALID_COMMANDS:
                await serial.send(cmd)
                await ws.send_json({"type": "ack", "cmd": cmd})
            else:
                await ws.send_json({"type": "error", "msg": f"Unknown command: {cmd}"})
    except WebSocketDisconnect:
        pass
    finally:
        clients.discard(ws)
        logger.info(f"Client disconnected ({len(clients)} total)")


@app.get("/")
async def index():
    return FileResponse(FRONTEND_DIR / "index.html")


app.mount("/", StaticFiles(directory=FRONTEND_DIR), name="frontend")
