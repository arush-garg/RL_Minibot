#include <ESP32Encoder.h>

#define pinA 18 
#define pinB 5

const int ENCODER_CPR = 1024;

ESP32Encoder encoder;

void setup () {
  encoder.attachHalfQuad(pinA, pinB);
  encoder.setCount(0);
  Serial.begin(115200);
}

void loop () {
  long newPosition = encoder.getCount();
  Serial.println((double)(newPosition*M_PI)/ENCODER_CPR);
  delay(25);
}