#include <Arduino.h>

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
  public:
      Robot(Motor leftFront, Motor rightFront, Motor leftBack, Motor rightBack): 
      leftFrontMotor{leftFront}, 
      rightFrontMotor{rightFront},
      leftBackMotor{leftBack},
      rightBackMotor{rightBack} { };
  
      void moveForward(int speed, float leftCoef = 1.0, float rightCoef = 1.0) {
        leftFrontMotor.forward(speed * leftCoef);
        rightFrontMotor.forward(speed * rightCoef);
        leftBackMotor.forward(speed * leftCoef);
        rightBackMotor.forward(speed * rightCoef);
      }

      void moveBackward(int speed, float leftCoef = 1.0, float rightCoef = 1.0) {
        leftFrontMotor.backward(speed * leftCoef);
        rightFrontMotor.backward(speed * rightCoef);
        leftBackMotor.backward(speed * leftCoef);
        rightBackMotor.backward(speed * rightCoef);
      }

      void stopMotors() {
          leftFrontMotor.stop();
          rightFrontMotor.stop();
          leftBackMotor.stop();
          rightBackMotor.stop();
      }
  
      void moveRight(int speed) {
        leftFrontMotor.forward(speed);
        rightFrontMotor.backward(speed);
        leftBackMotor.backward(speed);
        rightBackMotor.forward(speed);
      }
    
      void moveLeft(int speed) {
          leftFrontMotor.backward(speed);
          rightFrontMotor.forward(speed);
          leftBackMotor.forward(speed);
          rightBackMotor.backward(speed);
      }
  };

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
