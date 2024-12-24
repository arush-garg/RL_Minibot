#include <ESP32Encoder.h>
#include <math.h> 

const int ENCODER_CPR = 1024;

/*
  Returns the cumulative angle in radians
*/
double readEncoder() {
  int64_t position = encoder.getCount();
  double angle = (double)(position/ENCODER_CPR)*M_PI;
  return angle;
}