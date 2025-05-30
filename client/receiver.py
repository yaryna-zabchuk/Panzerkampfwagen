import asyncio
import websockets
import json

class Connection:
    def __init__(self, esp_ip, esp_port):
        self.esp_ip = esp_ip
        self.esp_port = esp_port

    async def connect(self):
        return await websockets.connect()

    async def receive_data(self):    
        try:
            async for request in self.websocket:
                print(f"Request received: {request}")
                self.requests_queue.put_nowait(request)
        except websockets.exceptions.ConnectionClosed:
            print("Websocket was closed")
            await self.disconnect()
            return
        except asyncio.CancelledError:
            print("Handler task was cancelled")
            return
    
    async def send_data(self, data: dict):
        try:
            data = json.dumps(data).encode('utf-8')
            await self.websocket.send(data)
        except websockets.exceptions.ConnectionClosed:
            print("Websocket was closed")
            await self.disconnect()
            return
        except asyncio.CancelledError:
            print("Handler task was cancelled")
            return