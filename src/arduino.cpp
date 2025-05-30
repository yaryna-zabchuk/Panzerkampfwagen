#ifdef ARDUINO_UNO

#include <Arduino.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>

class Coordinates {
  private:
    float x;
    float y;
    float angle;
    char lastMove;
    unsigned long lastTime;
    float velocity; 
    float angle_velocity;

  public:
    Coordinates() {
      x = 0.0;
      y = 0.0;
      angle = 0.0;
      lastMove = 's'; // 's' for stop, або інше значення за замовчуванням
      lastTime = millis();
      velocity = 1.0; // Початкова швидкість 0
      angle_velocity = 1.0; // Початкова кутова швидкість 0
    }
    
    void setLastMove(char newMove) {
      lastMove = newMove;
    }

    void updateCoordinates() {
      float currentTime = millis();
      float delta_t = (currentTime - lastTime) / 1000.0;
      lastTime = currentTime;

      if (lastMove == 'f') {
        float delta = velocity * delta_t;
        x += delta * cos(angle);
        y += delta * sin(angle);
      } else if (lastMove == 'b') {
        float delta = velocity * delta_t;
        x -= delta * cos(angle);
        y -= delta * sin(angle);
      } else if (lastMove == 'r') {
        angle -= angle_velocity * delta_t;
      } else if (lastMove == 'l') {
        angle += angle_velocity * delta_t;
      }
    }
};

class UltraSonic {
  private:
    int trigPin;
    int echoPin;

  public:
    UltraSonic(int trig, int echo) {
      trigPin = trig;
      echoPin = echo;
      pinMode(trigPin, OUTPUT);
      pinMode(echoPin, INPUT);
    }

    // Метод вимірювання відстані в сантиметрах
    float getDistance() {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      unsigned long duration = pulseIn(echoPin, HIGH, 8746);

      float distance = (duration * 0.0343) / 2.0;

      return (distance > 0.0) ? distance : 100.0;
    }
};


class Motor {
  private:
      int analogPin;
      int frontPin;
      int backPin;
  public:
      Motor(int anPin, int frPin, int bcPin) {
          analogPin = anPin;
          frontPin = frPin;
          backPin = bcPin;
          pinMode(analogPin, OUTPUT);
          pinMode(frontPin, OUTPUT);
          pinMode(backPin, OUTPUT);
      };
  
      void forward(int speed) {
          analogWrite(analogPin, speed);
          digitalWrite(frontPin, HIGH);
          digitalWrite(backPin, LOW);
      }
  
      void stop() {
          analogWrite(analogPin, 0);
          digitalWrite(frontPin, LOW);
          digitalWrite(backPin, LOW);
      }
  
      void backward(int speed) {
          analogWrite(analogPin, speed);
          digitalWrite(frontPin, LOW);
          digitalWrite(backPin, HIGH);
      }
  };

  class Robot {
    private:
        Motor leftMotor;
        Motor rightMotor;
        Coordinates coordinates;
    public:
        Robot(Motor leftM, Motor rightM, Coordinates currentCoordinates): leftMotor{leftM}, rightMotor{rightM}, coordinates{currentCoordinates}{ };
    
        void moveForward(int speed, float left, float right) {
            leftMotor.forward(speed * left);
            rightMotor.forward(speed * right);
            coordinates.setLastMove('f');
        }
    
        void stopMotors() {
            leftMotor.stop();
            rightMotor.stop();
            coordinates.setLastMove('s');
        }
    
        void rotateClockwise(int speed) {
            leftMotor.forward(speed);  // Лівий мотор рухається вперед
            rightMotor.backward(speed);    // Правий мотор рухається назад
            coordinates.setLastMove('r');
        }
    
        void rotateCounterClockwise(int speed) {
            leftMotor.backward(speed);     // Лівий мотор рухається назад
            rightMotor.forward(speed); // Правий мотор рухається вперед
            coordinates.setLastMove('l');
        }
    
        void moveBackward(int speed, float left, float right) {
            leftMotor.backward(speed * left);  // Лівий мотор рухається назад з урахуванням коефіцієнта
            rightMotor.backward(speed * right); // Правий мотор рухається назад з урахуванням коефіцієнта
            coordinates.setLastMove('b');
        }
    };


Motor lMotor(5, 9, 10);
Motor rMotor(6, 12, 11);
Robot robot(lMotor, rMotor);

// Variable to store current movement state
char currentMovement = 's'; // Default to stopped
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 3000; // 3 seconds timeout for safety

void setup() {
    Serial.begin(115200);

    Wire.begin();
    mpu.initialize();

    if (mpu.testConnection()) {
        Serial.println("MPU6050 connected successfully.");
    } else {
        Serial.println("MPU6050 connection failed.");
    }
    
    // Initialize all pins
    pinMode(5, OUTPUT);  // Left motor analog pin
    pinMode(6, OUTPUT);  // Right motor analog pin
    pinMode(9, OUTPUT);  // Left motor front pin
    pinMode(10, OUTPUT); // Left motor back pin
    pinMode(11, OUTPUT); // Right motor back pin
    pinMode(12, OUTPUT); // Right motor front pin
    
    // Start with motors stopped
    robot.stopMotors();
}

void executeMovement(char command) {
    switch(command) {
        case 'f':
            robot.moveForward(255, 1, 1);
            Serial.println("Moving forward");
            break;
        case 'b':
            robot.moveBackward(255, 1, 1);
            Serial.println("Moving backward");
            break;
        case 'l':
            robot.rotateCounterClockwise(255);
            Serial.println("Rotating counter-clockwise");
            break;
        case 'r':
            robot.rotateClockwise(255);
            Serial.println("Rotating clockwise");
            break;
        case 's':
        default:
            robot.stopMotors();
            Serial.println("Stopping motors");
            break;
    }
}

void loop() {
    // Check for new commands
    if (Serial.available()) {
        char newCommand = Serial.read();
        
        // Filter valid commands only
        if (newCommand == 'f' || newCommand == 'b' || 
            newCommand == 'l' || newCommand == 'r' || newCommand == 's') {
            
            // Only update if command has changed
            if (newCommand != currentMovement) {
                currentMovement = newCommand;
                executeMovement(currentMovement);
            }
            
            lastCommandTime = millis();
        }
        
        // Consume any additional bytes in the buffer
        while (Serial.available()) {
            Serial.read();
        }
    }
    
    // Safety feature: stop if no commands received for a while
    if (millis() - lastCommandTime > COMMAND_TIMEOUT && currentMovement != 's') {
        currentMovement = 's';
        executeMovement(currentMovement);
        Serial.println("Command timeout - stopping for safety");
    }
    
    // Optional: Check for obstacles or other sensor data here
    // and override movement if necessary
}

#endif