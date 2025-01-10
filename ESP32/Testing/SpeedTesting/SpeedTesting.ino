#include <ESP32Servo.h>

int leftMotorPin = 13;
int rightMotorPin = 14;

int buttonPin = 21;

Servo rightMotor;
Servo leftMotor;

bool started = false;

void setup() {
  Serial.begin(115200);
  rightMotor.attach(rightMotorPin);
  leftMotor.attach(leftMotorPin);

  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  if(digitalRead(buttonPin) == LOW) {
    started = true;
  }
  
  if(started) {
    rightMotor.writeMicroseconds(2000);
    leftMotor.writeMicroseconds(1000);
  }
}