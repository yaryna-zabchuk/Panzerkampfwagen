from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QPushButton, 
                            QLabel, QGridLayout, QSizePolicy, QFrame)
from PyQt5.QtCore import QTimer, Qt, pyqtSlot
from PyQt5.QtGui import QKeyEvent, QFont, QColor
import pyqtgraph as pg
import asyncio
from math import cos, sin, radians
import numpy as np


ROBOT_SPEED = 0.75  # m/s
ROTATION_SPEED = 0.75  # rad/s
KEYBOARD_READ_INTERVAL = 50  # ms
MAP_SIZE = 4  # Total map size in meters (reduced from 6)

class ArrowButton(QPushButton):
    def __init__(self, direction, parent=None):
        super().__init__(parent)
        self.direction = direction
        self.active = False
        
        # Set arrow symbols
        if direction == "forward":
            self.setText("↑")
        elif direction == "backward":
            self.setText("↓")
        elif direction == "left":
            self.setText("←")
        elif direction == "right":
            self.setText("→")
            
        # Set size and style
        self.setFixedSize(50, 50)
        self.setFont(QFont('Arial', 25))
        self.setStyleSheet("QPushButton { background-color: lightgray; }")
        
    def set_active(self, active):
        self.active = active
        if active:
            self.setStyleSheet("QPushButton { background-color: #90EE90; }")
        else:
            self.setStyleSheet("QPushButton { background-color: lightgray; }")

class MineMap(QWidget):
    def __init__(self, connection=None):
        super().__init__()
        self.connection = connection
        self.key_pressed = None  # Track currently pressed key
        
        self.setWindowTitle("Mine Detection Map")
        self.resize(800, 600)
        self.setStyleSheet("background-color: #7c8087;")

        # Main layout with spacing
        main_layout = QHBoxLayout()
        main_layout.setSpacing(10)
        main_layout.setContentsMargins(10, 10, 10, 10)
        self.setLayout(main_layout)
        
        # Left panel - Arrow controls
        left_panel = QWidget()
        left_layout = QGridLayout()
        left_layout.setSpacing(5)
        left_panel.setLayout(left_layout)
        left_panel.setFixedWidth(275)  # Increased width for better spacing
        
        # Create arrow buttons
        self.forward_btn = ArrowButton("forward")
        self.backward_btn = ArrowButton("backward")
        self.left_btn = ArrowButton("left")
        self.right_btn = ArrowButton("right")
        
        # Add buttons to grid
        left_layout.addWidget(self.forward_btn, 0, 1)
        left_layout.addWidget(self.left_btn, 1, 0)
        left_layout.addWidget(self.right_btn, 1, 2)
        left_layout.addWidget(self.backward_btn, 2, 1)
        
        # Controls label
        controls_label = QLabel("Controls (WASD)")
        controls_label.setAlignment(Qt.AlignCenter)
        left_layout.addWidget(controls_label, 3, 0, 1, 3)
        
        # Key legend
        key_info = QLabel("W - Forward\nS - Backward\nA - Turn Left\nD - Turn Right\nM - Place Mine")
        key_info.setAlignment(Qt.AlignCenter)
        left_layout.addWidget(key_info, 4, 0, 1, 3)
        
        # Center - Map
        center_panel = QWidget()
        center_layout = QVBoxLayout()
        center_layout.setContentsMargins(10, 10, 10, 10)
        center_panel.setLayout(center_layout)
        
        # Create a frame to contain the map with fixed square size
        map_frame = QFrame()
        map_frame.setFrameShape(QFrame.NoFrame)
        map_frame.setFixedSize(1450, 1450)  # Fixed square size for map
        map_layout = QVBoxLayout(map_frame)
        map_layout.setContentsMargins(0, 0, 0, 0)
        
        # Create the plot with -2 to 2 range (smaller map)
        self.plot = pg.PlotWidget()
        self.plot.setXRange(-2, 2)
        self.plot.setYRange(-2, 2)
        self.plot.setAspectLocked(True)  # Keep square scale
        map_layout.addWidget(self.plot)
        
        # Add frame to center layout with alignment
        center_layout.addStretch(1)
        center_layout.addWidget(map_frame, 0, Qt.AlignCenter)
        
        # Map title
        map_label = QLabel("Mine Detection Map")
        map_label.setAlignment(Qt.AlignCenter)
        center_layout.addWidget(map_label)
        center_layout.addStretch(1)
        
        # Right panel - Mine marker button
        right_panel = QWidget()
        right_layout = QVBoxLayout()
        right_panel.setLayout(right_layout)
        right_panel.setFixedWidth(275)  # Increased width for better spacing
        
        # Add some space at the top
        right_layout.addSpacing(10)
        
        self.add_mine_btn = QPushButton("Add Mine Marker")
        self.add_mine_btn.setFixedHeight(50)
        self.add_mine_btn.clicked.connect(self.place_mine_at_robot)
        right_layout.addWidget(self.add_mine_btn)
        
        # Status label for mine placement
        self.mine_status = QLabel("Last mine added: None")
        self.mine_status.setWordWrap(True)
        self.mine_status.setAlignment(Qt.AlignCenter)
        right_layout.addWidget(self.mine_status)
        
        # Mine count label
        self.mine_count = QLabel("Total mines: 0")
        self.mine_count.setAlignment(Qt.AlignCenter)
        right_layout.addWidget(self.mine_count)
        
        # Position display
        self.position_display = QLabel("Position: (0.00, 0.00)")
        self.position_display.setAlignment(Qt.AlignCenter)
        right_layout.addWidget(self.position_display)
        
        right_layout.addStretch()
        
        # Add panels to main layout
        main_layout.addWidget(left_panel)
        main_layout.addWidget(center_panel, 1)  # Give center panel stretch priority
        main_layout.addWidget(right_panel)

        # Add coordinate axes (cross)
        self.x_axis = self.plot.plot([-2, 2], [0, 0], pen=pg.mkPen('black', width=1))
        self.y_axis = self.plot.plot([0, 0], [-2, 2], pen=pg.mkPen('black', width=1))
        
        # Add axis labels
        x_label = pg.TextItem("X", anchor=(0.5, 0))
        y_label = pg.TextItem("Y", anchor=(0, 0.5))
        x_label.setPos(2, 0.1)
        y_label.setPos(0.1, 2)
        self.plot.addItem(x_label)
        self.plot.addItem(y_label)

        # Data: robot + mines
        self.robot_pos = [0, 0]  # Center of the map (origin)
        self.robot_angle = 0  # In radians
        self.mines = []

        # Plot items
        self.robot_dot = self.plot.plot([self.robot_pos[0]], [self.robot_pos[1]], 
                                       pen=None, symbol='o', symbolBrush='blue', symbolSize=20)
        
        # Add direction indicator (line showing robot heading)
        self.direction_line = self.plot.plot([0, 0], [0, 0], pen=pg.mkPen('blue', width=2))
        
        # Add grid lines
        self.plot.showGrid(x=True, y=True)
        
        self.mine_dots = []

        # Timer to refresh display
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_gui)
        self.timer.start(100)  # ms
        
        # Timer to send direction commands
        self.command_timer = QTimer()
        self.command_timer.timeout.connect(self.send_direction)
        self.command_timer.start(KEYBOARD_READ_INTERVAL)
    
    def place_mine_at_robot(self):
        """Place a mine marker at the current robot position"""
        x, y = self.robot_pos[0], self.robot_pos[1]
        self.add_mine(x, y)
        self.mine_status.setText(f"Last mine added: ({x:.2f}, {y:.2f})")
        self.mine_count.setText(f"Total mines: {len(self.mines)}")
        print(f"Mine added at robot position: ({x:.2f}, {y:.2f})")
    
    def keyPressEvent(self, event: QKeyEvent):
        prev_key = self.key_pressed
        
        if event.key() == Qt.Key_W:
            self.key_pressed = "forward"
            self.forward_btn.set_active(True)
            new_x = self.robot_pos[0] + ROBOT_SPEED*cos(self.robot_angle)*KEYBOARD_READ_INTERVAL/1000
            new_y = self.robot_pos[1] + ROBOT_SPEED*sin(self.robot_angle)*KEYBOARD_READ_INTERVAL/1000
            # Check map boundaries (-2 to 2 range)
            if -2 <= new_x <= 2 and -2 <= new_y <= 2:
                self.robot_pos[0] = new_x
                self.robot_pos[1] = new_y
        elif event.key() == Qt.Key_S:
            self.key_pressed = "backward"
            self.backward_btn.set_active(True)
            new_x = self.robot_pos[0] - ROBOT_SPEED*cos(self.robot_angle)*KEYBOARD_READ_INTERVAL/1000
            new_y = self.robot_pos[1] - ROBOT_SPEED*sin(self.robot_angle)*KEYBOARD_READ_INTERVAL/1000
            # Check map boundaries (-2 to 2 range)
            if -2 <= new_x <= 2 and -2 <= new_y <= 2:
                self.robot_pos[0] = new_x
                self.robot_pos[1] = new_y
        elif event.key() == Qt.Key_A:
            self.key_pressed = "left"
            self.left_btn.set_active(True)
            self.robot_angle += (ROTATION_SPEED * (KEYBOARD_READ_INTERVAL / 1000))
        elif event.key() == Qt.Key_D: 
            self.key_pressed = "right"
            self.right_btn.set_active(True)
            self.robot_angle -= (ROTATION_SPEED * (KEYBOARD_READ_INTERVAL / 1000))
        elif event.key() == Qt.Key_M:
            # Add mine at robot position when M is pressed
            self.place_mine_at_robot()
        
        # Keep angle in sensible range
        while self.robot_angle > 2*np.pi:
            self.robot_angle -= 2*np.pi
        while self.robot_angle < 0:
            self.robot_angle += 2*np.pi
            
        self.update_robot_position(self.robot_pos[0], self.robot_pos[1])
        print(f"Position: ({self.robot_pos[0]:.2f}, {self.robot_pos[1]:.2f}), Angle: {self.robot_angle:.2f} rad")
        print(f"Command: {self.key_pressed}")

    def keyReleaseEvent(self, event: QKeyEvent):
        # Only clear if this key was the active one
        if event.key() == Qt.Key_W and self.key_pressed == "forward":
            self.key_pressed = None
            self.forward_btn.set_active(False)
        elif event.key() == Qt.Key_S and self.key_pressed == "backward":
            self.key_pressed = None
            self.backward_btn.set_active(False)
        elif event.key() == Qt.Key_A and self.key_pressed == "left":
            self.key_pressed = None
            self.left_btn.set_active(False)
        elif event.key() == Qt.Key_D and self.key_pressed == "right":
            self.key_pressed = None
            self.right_btn.set_active(False)

    def send_direction(self):
        if self.connection and self.connection.is_websocket_open.is_set():
            direction = self.key_pressed if self.key_pressed else "none"
            asyncio.create_task(self.connection.send_data({"cmd": direction}))

    def update_gui(self):
        # Update robot position
        self.robot_dot.setData([self.robot_pos[0]], [self.robot_pos[1]])
        
        # Update direction indicator line
        direction_length = 0.3  # Length of the direction indicator (reduced)
        end_x = self.robot_pos[0] + direction_length * cos(self.robot_angle)
        end_y = self.robot_pos[1] + direction_length * sin(self.robot_angle)
        self.direction_line.setData(
            [self.robot_pos[0], end_x],
            [self.robot_pos[1], end_y]
        )
        
        # Update position display
        self.position_display.setText(f"Position: ({self.robot_pos[0]:.2f}, {self.robot_pos[1]:.2f})")

        # Clear old mine markers
        for dot in self.mine_dots:
            self.plot.removeItem(dot)
        self.mine_dots.clear()

        # Redraw mine markers
        for (x, y) in self.mines:
            dot = self.plot.plot([x], [y], pen=None, symbol='x', symbolBrush='red', symbolSize=15)
            self.mine_dots.append(dot)

    def update_robot_position(self, x, y):
        self.robot_pos = [x, y]

    def add_mine(self, x, y):
        self.mines.append((x, y))