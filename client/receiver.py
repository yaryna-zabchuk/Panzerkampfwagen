import asyncio
import websockets
import json


class Connection:
    def __init__(self, esp_ip, esp_port):
        self.uri = f"ws://{esp_ip}:{esp_port}"
        self.data_queue = asyncio.Queue()

    async def __connect(self):
        self.websocket = await websockets.connect(self.uri)
        print(f"Connected to {self.uri}")

    async def __receive_data(self):
        try:
            async for request in self.websocket:
                # print(f"Request received: {request}")
                self.data_queue.put_nowait(request)
        except websockets.exceptions.ConnectionClosed:
            print("Websocket was closed")
            await self.websocket.close()
            return
        except asyncio.CancelledError:
            print("Handler task was cancelled")
            return

    async def __process_data (self):
        while True:
            try:
                data = await self.data_queue.get()
                print(f"Processing data: {data}")
            except asyncio.CancelledError:
                print("Data processing task was cancelled")
                break

    async def send_data(self, data: dict):
        try:
            data = json.dumps(data).encode('utf-8')
            await self.websocket.send(data)
        except websockets.exceptions.ConnectionClosed:
            print("Websocket was closed")
            await self.websocket.close()
            return
        except asyncio.CancelledError:
            print("Handler task was cancelled")
            return

    async def run(self):
        await self.__connect()
        receive_task = asyncio.create_task(self.__receive_data())
        process_task = asyncio.create_task(self.__process_data())

        try:
            await asyncio.gather(receive_task, process_task)
        except asyncio.CancelledError:
            print("Run task was cancelled")
        finally:
            await self.websocket.close()
