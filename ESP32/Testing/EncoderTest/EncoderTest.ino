#include <ESP32Encoder.h>

#define pinA 18 // CLK ENCODER
#define pinB 5 // DT ENCODER

const int ENCODER_CPR = 1024;

ESP32Encoder encoder;

void setup () {
  encoder.attachHalfQuad(pinA, pinB);
  encoder.setCount(0);
  Serial.begin(115200);
}

void loop () {
  long newPosition = encoder.getCount();
  Serial.println((double)(newPosition)/ENCODER_CPR);
  delay(25);
}