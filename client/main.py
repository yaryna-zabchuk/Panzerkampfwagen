# import sys
# from PyQt5.QtWidgets import QApplication
# from gui import MineMap
from receiver import Connection
from config import ESP_IP, ESP_PORT
import asyncio


async def main():
    connection = Connection(ESP_IP, ESP_PORT)
    await connection.run()

if __name__ == "__main__":
    asyncio.run(main())




# app = QApplication(sys.argv)
# window = MineMap()
# window.show()
