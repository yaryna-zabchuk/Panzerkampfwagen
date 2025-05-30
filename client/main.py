import sys
from PyQt5.QtWidgets import QApplication
from gui import MineMap


app = QApplication(sys.argv)
window = MineMap()
window.show()