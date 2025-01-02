#include <ESP32Servo.h>


int rightMotorPin = 14;
int leftMotorPin = 13;

bool done = false;
long startTime;

Servo rightMotor;
Servo leftMotor;

void setup() {
  Serial.begin(115200);

  // put your setup code here, to run once:
  rightMotor.attach(rightMotorPin);
  leftMotor.attach(leftMotorPin);

  startTime = millis();
  Serial.println(millis()-startTime);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!done && millis() - startTime < 1000) {
    rightMotor.writeMicroseconds(1000);
    leftMotor.writeMicroseconds(2000);
  }
  else {
    done = true;
    rightMotor.writeMicroseconds(1500);
    leftMotor.writeMicroseconds(1500);
    Serial.println("Milliseconds: "+(millis()-startTime));
  }
}
