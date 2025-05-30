import sys
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout
from PyQt5.QtCore import QTimer, Qt
from PyQt5.QtGui import QKeyEvent
import pyqtgraph as pg


class MineMap(QWidget):
    def __init__(self):
        super().__init__()

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
    
    def keyPressEvent(self, event: QKeyEvent):
        if event.key() == Qt.Key_W:
            print("W pressed")
        elif event.key() == Qt.Key_S:
            print("S pressed")
        if event.key() == Qt.Key_A:
            print("A pressed")
        elif event.key() == Qt.Key_D:
            print("D pressed")
        else:
            print(f"Other key: {event.text()}")

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

# # Example usage
# if __name__ == "__main__":
#     app = QApplication(sys.argv)
#     window = MineMap()
#     window.show()

#     # DEMO: simulate robot movement and mine detection
#     import random

#     def simulate():
#         x = random.uniform(0, 100)
#         y = random.uniform(0, 100)
#         window.update_robot_position(x, y)
#         if random.random() < 0.2:  # 20% chance to detect mine
#             window.add_mine(x, y)

#     sim_timer = QTimer()
#     sim_timer.timeout.connect(simulate)
#     sim_timer.start(500)  # Simulate every 500 ms

#     sys.exit(app.exec_())
