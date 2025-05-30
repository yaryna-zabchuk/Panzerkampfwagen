#include <Arduino.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>

MPU6050 mpu;

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
      Motor leftFrontMotor;
      Motor rightFrontMotor;
      Motor leftBackMotor;
      Motor rightBackMotor;
      Coordinates coordinates;
  public:
      // Конструктор класу Robot
      Robot(Motor leftFront, Motor rightFront, Motor leftBack, Motor rightBack, Coordinates currentCoordinates): 
      leftFrontMotor{leftFront}, 
      rightFrontMotor{rightFront},
      leftBackMotor{leftBack},
      rightBackMotor{rightBack}, 
      coordinates{currentCoordinates}{ };
  
      void moveForward(int speed, float leftCoef = 1.0, float rightCoef = 1.0) {
        leftFrontMotor.forward(speed * leftCoef);
        rightFrontMotor.forward(speed * rightCoef);
        leftBackMotor.forward(speed * leftCoef);
        rightBackMotor.forward(speed * rightCoef);
        coordinates.setLastMove('f');
      }

      void moveBackward(int speed, float leftCoef = 1.0, float rightCoef = 1.0) {
        leftFrontMotor.backward(speed * leftCoef);
        rightFrontMotor.backward(speed * rightCoef);
        leftBackMotor.backward(speed * leftCoef);
        rightBackMotor.backward(speed * rightCoef);
        coordinates.setLastMove('b');
      }

      void stopMotors() {
          leftFrontMotor.stop();
          rightFrontMotor.stop();
          leftBackMotor.stop();
          rightBackMotor.stop();
          coordinates.setLastMove('s');
      }
  
      void moveRight(int speed) {
        leftFrontMotor.forward(speed);
        rightFrontMotor.backward(speed);
        leftBackMotor.backward(speed);
        rightBackMotor.forward(speed);
        coordinates.setLastMove('r');
      }
    
      void moveLeft(int speed) {
          leftFrontMotor.backward(speed);
          rightFrontMotor.forward(speed);
          leftBackMotor.forward(speed);
          rightBackMotor.backward(speed);
          coordinates.setLastMove('l');
      }
  };

void setup() {
    Serial.begin(9600);
    Wire.begin();
    mpu.initialize();

    if (mpu.testConnection()) {
        Serial.println("MPU6050 connected successfully.");
    } else {
        Serial.println("MPU6050 connection failed.");
    }
}

void loop() {
  // put your main code here, to run repeatedly:


}
