from PyQt5.QtWidgets import QWidget, QVBoxLayout
from PyQt5.QtCore import QTimer, Qt
from PyQt5.QtGui import QKeyEvent
import pyqtgraph as pg
import asyncio


class MineMap(QWidget):
    def __init__(self, connection=None):
        super().__init__()
        self.connection = connection
        self.key_pressed = None  # Track currently pressed key
        
        self.setWindowTitle("Mine Detection Map")
        self.resize(600, 600)

        layout = QVBoxLayout()
        self.setLayout(layout)

        # Create the plot
        self.plot = pg.PlotWidget()
        self.plot.setXRange(0, 100)
        self.plot.setYRange(0, 100)
        self.plot.setAspectLocked(True)  # Keep square scale
        layout.addWidget(self.plot)

        # Data: robot + mines
        self.robot_pos = [0, 0]
        self.mines = []

        # Plot items
        self.robot_dot = self.plot.plot([0], [0], pen=None, symbol='o', symbolBrush='blue', symbolSize=12)
        self.mine_dots = []

        # Timer to refresh display
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_gui)
        self.timer.start(100)  # ms
        
        # Timer to send direction commands
        self.command_timer = QTimer()
        self.command_timer.timeout.connect(self.send_direction)
        self.command_timer.start(200)
    
    def keyPressEvent(self, event: QKeyEvent):
        if event.key() == Qt.Key_W:
            self.key_pressed = "forward"
        elif event.key() == Qt.Key_S:
            self.key_pressed = "backward"
        elif event.key() == Qt.Key_A:
            self.key_pressed = "right" # Inverted direction
        elif event.key() == Qt.Key_D: 
            self.key_pressed = "left" # Inverted direction
        print(self.key_pressed)
    
    def keyReleaseEvent(self, event: QKeyEvent):
        # Only clear if this key was the active one
        if ((event.key() == Qt.Key_W and self.key_pressed == "forward") or
            (event.key() == Qt.Key_S and self.key_pressed == "backward") or
            (event.key() == Qt.Key_A and self.key_pressed == "right") or # Inverted direction
            (event.key() == Qt.Key_D and self.key_pressed == "left")): # Inverted direction
            self.key_pressed = "none"

    def send_direction(self):
        if self.connection and self.connection.is_websocket_open.is_set():
            direction = self.key_pressed if self.key_pressed else "none"
            asyncio.create_task(self.connection.send_data({"cmd": direction}))

    def update_gui(self):
        # Update robot position
        self.robot_dot.setData([self.robot_pos[0]], [self.robot_pos[1]])

        # Clear old mine markers
        for dot in self.mine_dots:
            self.plot.removeItem(dot)
        self.mine_dots.clear()

        # Redraw mine markers
        for (x, y) in self.mines:
            dot = self.plot.plot([x], [y], pen=None, symbol='x', symbolBrush='red', symbolSize=10)
            self.mine_dots.append(dot)

    def update_robot_position(self, x, y):
        self.robot_pos = [x, y]

    def add_mine(self, x, y):
        self.mines.append((x, y))
