import asyncio
import cbor2
import hashlib
from bleak import BleakClient, BleakScanner
import argparse

# Opcodes
MGMT_OP_READ = 0
MGMT_OP_READ_RSP = 1
MGMT_OP_WRITE = 2
MGMT_OP_WRITE_RSP = 3

# Groups
MGMT_GROUP_ID_OS = 0
MGMT_GROUP_ID_IMAGE = 1

# OS group
OS_MGMT_ID_RESET = 5

# Image group
IMG_MGMT_ID_STATE = 0
IMG_MGMT_ID_UPLOAD = 1
IMG_MGMT_ID_ERASE = 5


class MCUManager:
    SERVICE_UUID = "8d53dc1d-1db7-4cd3-868b-8a527460aa84"
    CHARACTERISTIC_UUID = "da2e7828-fbce-4e01-ae9e-261174997c48"

    def __init__(self, logger=None, mtu=180):
        self._mtu = mtu
        self._device = None
        self._client: BleakClient = None
        self._characteristic = None
        self._seq = 0
        self._buffer = bytearray()
        self._uploadIsInProgress = False
        self._uploadOffset = 0
        self._uploadImage = None
        self._imageIdx = 0
        self._uploadTimeout = None
        self._userRequestedDisconnect = False
        self._logger = logger or {"info": print, "error": print}

        self._lastImageState = None  # 保存镜像信息

        # Callbacks
        self._connectCallback = None
        self._connectingCallback = None
        self._disconnectCallback = None
        self._messageCallback = None
        self._imageUploadProgressCallback = None
        self._imageUploadFinishedCallback = None

    async def _request_device(self, name_filter=None):
        devices = await BleakScanner.discover()
        for d in devices:
            if not d.name:
                continue
            if not name_filter or name_filter in d.name:
                self._logger["info"](f"Found device: {d.name}, {d.address}")
                return d
        raise RuntimeError("Device not found")

    async def connect(self, name_filter=None):
        self._device = await self._request_device(name_filter)
        self._client = BleakClient(self._device)
        try:
            if self._connectingCallback:
                self._connectingCallback()
            await self._client.connect()
            self._logger["info"]("Device connected")
            self._characteristic = self.CHARACTERISTIC_UUID
            await self._client.start_notify(
                self._characteristic, self._notification
            )
            if self._connectCallback:
                self._connectCallback()
        except Exception as e:
            self._logger["error"](e)

    async def disconnect(self):
        self._userRequestedDisconnect = True
        if self._client:
            await self._client.disconnect()
            if self._disconnectCallback:
                self._disconnectCallback()
            self._logger["info"]("Disconnected")

    # Callbacks
    def onConnecting(self, callback):
        self._connectingCallback = callback
        return self

    def onConnect(self, callback):
        self._connectCallback = callback
        return self

    def onDisconnect(self, callback):
        self._disconnectCallback = callback
        return self

    def onMessage(self, callback):
        self._messageCallback = callback
        return self

    def onImageUploadProgress(self, callback):
        self._imageUploadProgressCallback = callback
        return self

    def onImageUploadFinished(self, callback):
        self._imageUploadFinishedCallback = callback
        return self

    @property
    def name(self):
        return self._device.name if self._device else None

    async def _sendMessage(self, op, group, id, data=None):
        _flags = 0
        encodedData = cbor2.dumps(data) if data is not None else b""
        length_lo = len(encodedData) & 0xFF
        length_hi = len(encodedData) >> 8
        group_lo = group & 0xFF
        group_hi = group >> 8

        message = bytearray([op, _flags, length_hi, length_lo, group_hi, group_lo, self._seq, id])
        message.extend(encodedData)

        # 按 MTU 拆包发送
        for i in range(0, len(message), self._mtu):
            chunk = message[i:i + self._mtu]
            await self._client.write_gatt_char(self._characteristic, chunk)
            await asyncio.sleep(0.01)  # 避免发送过快

        self._seq = (self._seq + 1) % 256

    async def _uploadNext(self):
        if self._uploadOffset >= len(self._uploadImage):
            self._uploadIsInProgress = False
            if self._imageUploadFinishedCallback:
                self._imageUploadFinishedCallback()
            return

        message = {
            "image": self._imageIdx,
            "data": b"",
            "off": self._uploadOffset,
        }
        if self._uploadOffset == 0:
            message["len"] = len(self._uploadImage)
            message["sha"] = await self._hash(self._uploadImage)

        if self._imageUploadProgressCallback:
            self._imageUploadProgressCallback(
                {"percentage": int(self._uploadOffset / len(self._uploadImage) * 100)}
            )

        # 计算本次 chunk 尺寸，保证 CBOR 编码 + header 不超过 MTU
        nmpOverhead = 8
        maxDataLength = self._mtu - nmpOverhead - len(cbor2.dumps({k:v for k,v in message.items() if k != "data"}))
        message["data"] = self._uploadImage[self._uploadOffset:self._uploadOffset + maxDataLength]

        if self._uploadTimeout:
            self._uploadTimeout.cancel()

        loop = asyncio.get_running_loop()
        self._uploadTimeout = loop.call_later(2.0, lambda: asyncio.create_task(self._retryChunk()))

        await self._sendMessage(MGMT_OP_WRITE, MGMT_GROUP_ID_IMAGE, IMG_MGMT_ID_UPLOAD, message)

    async def _retryChunk(self):
        self._logger["info"]("Upload chunk timeout, retrying...")
        await self._uploadNext()

    async def _hash(self, data):
        return hashlib.sha256(data).digest()

    def _notification(self, sender, data: bytearray):
        self._buffer.extend(data)
        while len(self._buffer) >= 8:
            messageLength = self._buffer[2] * 256 + self._buffer[3]
            if len(self._buffer) < messageLength + 8:
                return
            msg = self._buffer[: messageLength + 8]
            self._processMessage(msg)
            self._buffer = self._buffer[messageLength + 8:]

    def _processMessage(self, message):
        op, _flags, length_hi, length_lo, group_hi, group_lo, _seq, id = message[:8]
        length = length_hi * 256 + length_lo
        group = group_hi * 256 + group_lo
        data = cbor2.loads(message[8:]) if length > 0 else {}
        if (
            group == MGMT_GROUP_ID_IMAGE
            and id == IMG_MGMT_ID_UPLOAD
            and (data.get("rc") == 0 or data.get("rc") is None)
            and "off" in data
        ):
            if self._uploadTimeout:
                self._uploadTimeout.cancel()
            self._uploadOffset = data["off"]
            asyncio.create_task(self._uploadNext())
            return

        if group == MGMT_GROUP_ID_IMAGE and id == IMG_MGMT_ID_STATE:
            self._lastImageState = data

        if self._messageCallback:
            self._messageCallback({"op": op, "group": group, "id": id, "data": data})

    # ================= Commands =================
    async def cmdReset(self):
        await self._sendMessage(MGMT_OP_WRITE, MGMT_GROUP_ID_OS, OS_MGMT_ID_RESET)

    async def cmdImageState(self):
        await self._sendMessage(MGMT_OP_READ, MGMT_GROUP_ID_IMAGE, IMG_MGMT_ID_STATE)

    async def cmdImageErase(self):
        await self._sendMessage(MGMT_OP_WRITE, MGMT_GROUP_ID_IMAGE, IMG_MGMT_ID_ERASE, {})

    async def cmdConfirmImage(self, hash_bytes: bytes, confirm=False):
        msg = {"hash": hash_bytes, "confirm": confirm}
        await self._sendMessage(MGMT_OP_WRITE, MGMT_GROUP_ID_IMAGE, IMG_MGMT_ID_STATE, msg)

    async def cmdUpload(self, image: bytes, imageIdx=0):
        if self._uploadIsInProgress:
            self._logger["error"]("Upload is already in progress")
            return
        self._uploadIsInProgress = True
        self._uploadOffset = 0
        self._uploadImage = image
        self._imageIdx = imageIdx
        await self._uploadNext()


# ================= main =================
async def main():
    parser = argparse.ArgumentParser(description="BLE Firmware Updater")
    parser.add_argument("device_name", type=str, help="BLE device name filter")
    parser.add_argument("--app", type=str, default="app_update.bin", help="App core firmware file")
    parser.add_argument("--net", type=str, default="net_core_app_update.bin", help="Net core firmware file")
    args = parser.parse_args()

    currentUpload = "Upload_App_Core"
    manager = MCUManager()

    manager.onConnect(lambda: print("Connected"))
    manager.onDisconnect(lambda: print("Disconnected"))
    manager.onImageUploadProgress(lambda prog: print(currentUpload, ":", prog))
    manager.onImageUploadFinished(lambda: print(currentUpload, ":", "{'percentage': 100}"))

    # 连接设备
    await manager.connect(name_filter=args.device_name)

    # 上传 APP Core
    currentUpload = "Upload_App_Core"
    with open(args.app, "rb") as f:
        data = f.read()
        await manager.cmdUpload(data, imageIdx=0)
    while manager._uploadIsInProgress:
        await asyncio.sleep(0.5)

    # 上传 NET Core
    currentUpload = "Upload_Net_Core"
    with open(args.net, "rb") as f:
        data = f.read()
        await manager.cmdUpload(data, imageIdx=1)
    while manager._uploadIsInProgress:
        await asyncio.sleep(0.5)

    # 获取镜像信息
    await manager.cmdImageState()
    await asyncio.sleep(2)

    # 统一确认
    if manager._lastImageState and "images" in manager._lastImageState:
        for img in manager._lastImageState["images"]:
            hash_bytes = img.get("hash")
            if isinstance(hash_bytes, list):
                hash_bytes = bytes(hash_bytes)
            if hash_bytes:
                print(f"Confirming image {hash_bytes.hex()}")
                await manager.cmdConfirmImage(hash_bytes, confirm=False)

    await asyncio.sleep(2)
    await manager.cmdReset()
    print("Reboot")
    await asyncio.sleep(10)
    await manager.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
