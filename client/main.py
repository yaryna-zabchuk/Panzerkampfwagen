import sys
import asyncio
from PyQt5.QtWidgets import QApplication
from gui import MineMap
from receiver import Connection
from config import ESP_IP, ESP_PORT
import asyncio
import qasync


async def main():
    # Create application
    app = QApplication(sys.argv)
    loop = qasync.QEventLoop(app)
    asyncio.set_event_loop(loop)

    # Set up WebSocket connection
    connection = Connection(ESP_IP, ESP_PORT)
    connection_task = asyncio.create_task(connection.run())
    
    await connection.is_websocket_open.wait()
    # Create and show GUI
    window = MineMap(connection)
    window.show()
    
    # Wait for connection to establish
    await connection.is_websocket_open.wait()
    print("WebSocket connection established")
    
    # Run event loop
    await loop.run_forever()


if __name__ == "__main__":
    asyncio.run(main())