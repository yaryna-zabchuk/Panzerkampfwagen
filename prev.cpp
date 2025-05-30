#ifdef ARDUINO_UNO

#include <Arduino.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>

MPU6050 mpu;

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
    public:
        Robot(Motor leftM, Motor rightM): leftMotor{leftM}, rightMotor{rightM} { };
    
        void moveForward(int speed, float left, float right) {
            leftMotor.forward(speed * left);
            rightMotor.forward(speed * right);
        }
    
        void stopMotors() {
            leftMotor.stop();
            rightMotor.stop();
        }
    
        void rotateClockwise(int speed) {
            leftMotor.forward(speed);  // Лівий мотор рухається вперед
            rightMotor.backward(speed);    // Правий мотор рухається назад
        }
    
        void rotateCounterClockwise(int speed) {
            leftMotor.backward(speed);     // Лівий мотор рухається назад
            rightMotor.forward(speed); // Правий мотор рухається вперед
        }
    
        void moveBackward(int speed, float left, float right) {
            leftMotor.backward(speed * left);  // Лівий мотор рухається назад з урахуванням коефіцієнта
            rightMotor.backward(speed * right); // Правий мотор рухається назад з урахуванням коефіцієнта
        }
    };


Motor lMotor(5, 9, 10);
Motor rMotor(6, 12, 11);
Robot robot(lMotor, rMotor);

void setup() {
    Serial.begin(115200);

    Wire.begin();
    mpu.initialize();

    if (mpu.testConnection()) {
        Serial.println("MPU6050 connected successfully.");
    } else {
        Serial.println("MPU6050 connection failed.");
    }
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c == 'f') {
        robot.moveForward(255, 1, 1);
        Serial.println("Moving forward");
    } else if (c == 'b') {
        robot.moveBackward(255, 1, 1);
        Serial.println("Moving backward");
    } else if (c == 'l') {
        robot.rotateCounterClockwise(255);
        Serial.println("Rotating counter-clockwise");
    } else if (c == 'r') {
        robot.rotateClockwise(255);
        Serial.println("Rotating clockwise");
    } else if (c == 's') {
        robot.stopMotors();
        Serial.println("Stopping motors");
    }
  }
}


#endif