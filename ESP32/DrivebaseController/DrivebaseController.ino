#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <ESP32Encoder.h>
#include <unordered_map>
#include <ArduinoJson.h>

using namespace std;


/*const char* ssid = "iPhone (2)";
const char* password = "handGrenade";*/

const char* ssid = "Techno";
const char* password = "90023560";

const double SPEED = 0.2; //UPDATE THIS VALUE

const double MAX_POS = 2.4; //In meters
const double MAX_ANG = 0.262; //In radians



// State variables as dictionaries
unordered_map<string, float> currentState;
unordered_map<string, float> previousState;

double action;
float reward = 0.0;
bool done = false;


long elapsedTime = millis();
double timeForward = 0;

int rightMotorPin = 15; //CHECK AND UPDATE
int leftMotorPin = 21; //CHECK AND UPDATE

Servo rightMotor;
Servo leftMotor;

ESP32Encoder encoder;

void setup() {
  Serial.begin(115200);

  initCurrentState();
  initPrevState();

  initWiFi();

  rightMotor.attach(rightMotorPin);
  leftMotor.attach(leftMotorPin);

  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encoder.attachHalfQuad(18, 19); //A and B respectively
  encoder.clearCount();
  
}

void loop() {
  //Get values for 4 variables
  if(WiFi.status() == WL_CONNECTED) {
    currentState["cartPos"] = getCartPos();
    currentState["cartVel"] = getCartVel();
    currentState["poleAng"] = readEncoder();
    currentState["poleVel"] = getPoleVel();

    StaticJsonDocument<200> doc;
    doc["current_state"] = (currentState["cartPos"], currentState["cartVel"], currentState["poleAng"], currentState["poleVel"]);
    doc["previous_state"] = (previousState["cartPos"], previousState["cartVel"], previousState["poleAng"], previousState["poleVel"]);
    doc["action"] = action;
    doc["reward"] = reward;
    action = getPrediction(doc);

  }

  //Go forward
  if(action < 0.5) {
    rightMotor.writeMicroseconds(1750);
    leftMotor.writeMicroseconds(1750);
    timeForward += ((millis()-elapsedTime)/1000);
  }
  //Go backward
  else {
    rightMotor.writeMicroseconds(1250);
    leftMotor.writeMicroseconds(1250);
    timeForward -= ((millis()-elapsedTime)/1000);
  }
  updatePreviousState();
  elapsedTime = millis();
}


void initWiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

double getCartPos() {
  return (SPEED * timeForward / 1000);
}

double getCartVel() {
  return 1000*(currentState["cartPos"]-previousState["cartPos"])/(millis()-elapsedTime);
}

double getPoleVel() {
  return 1000*(currentState["poleAng"]-previousState["poleAng"])/(millis()-elapsedTime);
}



void initCurrentState() {
  currentState["cartPos"] = 0.0;
  currentState["cartVel"] = 0.0;
  currentState["poleAng"] = 0.0;
  currentState["poleVel"] = 0.0;
}

void initPrevState() {
  previousState["cartPos"] = 0.0;
  previousState["cartVel"] = 0.0;
  previousState["poleAng"] = 0.0;
  previousState["poleVel"] = 0.0;
}

void updatePreviousState() {
  previousState["cartPos"] = currentState["cartPos"];
  previousState["cartVel"] = currentState["cartVel"];
  previousState["poleAng"] = currentState["poleAng"];
  previousState["poleVel"] = currentState["poleVel"];
}