import asyncio
import logging

logger = logging.getLogger(__name__)

try:
    import serial_asyncio
    HAS_SERIAL = True
except ImportError:
    HAS_SERIAL = False


class SerialConnection:
    def __init__(self):
        self._reader = None
        self._writer = None
        self._connected = False

    @property
    def connected(self) -> bool:
        return self._connected

    async def connect(self, port: str = "/dev/ttyUSB0", baudrate: int = 9600):
        if not HAS_SERIAL:
            logger.warning("pyserial-asyncio not available — running in dev mode")
            return

        try:
            self._reader, self._writer = await serial_asyncio.open_serial_connection(
                url=port, baudrate=baudrate
            )
            self._connected = True
            logger.info(f"Serial connected: {port} @ {baudrate}")
        except Exception as e:
            logger.warning(f"Serial connection failed ({e}) — running in dev mode")
            self._connected = False

    async def send(self, command: str):
        payload = (command.strip() + "\n").encode()
        if self._connected and self._writer:
            self._writer.write(payload)
            await self._writer.drain()
            logger.info(f"Serial TX: {command}")
        else:
            logger.info(f"Dev mode TX: {command}")

    async def read(self):
        if not self._connected or not self._reader:
            return None
        try:
            line = await asyncio.wait_for(self._reader.readline(), timeout=0.1)
            return line.decode().strip()
        except asyncio.TimeoutError:
            return None

    async def disconnect(self):
        if self._writer:
            self._writer.close()
            self._connected = False
            logger.info("Serial disconnected")
