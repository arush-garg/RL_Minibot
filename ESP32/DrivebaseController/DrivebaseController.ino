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

int action = -1;
float reward = 0.0;
bool done = false;


long elapsedTime;
double timeForward = 0;

int rightMotorPin = 14;
int leftMotorPin = 13;

int encoderPinA = 18;
int encoderPinB = 5; //CHECK AND UPDATE

Servo rightMotor;
Servo leftMotor;

ESP32Encoder encoder;


StaticJsonDocument<200> addCurrentState(StaticJsonDocument<200> doc) {
  JsonArray current_state = doc.createNestedArray("current_state");
  current_state.add(currentState["cartPos"]);
  current_state.add(currentState["cartVel"]);
  Serial.println(currentState["cartVel"]);
  current_state.add(currentState["poleAng"]);
  current_state.add(currentState["poleVel"]); 
  Serial.println(currentState["poleVel"]);
  return doc;  
}

StaticJsonDocument<200> addPreviousState(StaticJsonDocument<200> doc) {
  JsonArray prev_state = doc.createNestedArray("previous_state");
  prev_state.add(previousState["cartPos"]);
  prev_state.add(previousState["cartVel"]);
  prev_state.add(previousState["poleAng"]);
  prev_state.add(previousState["poleVel"]);   
  return doc;
}


void setup() {
  Serial.begin(115200);
  elapsedTime = millis();
  
  initCurrentState();
  initPrevState();

  initWiFi();

  rightMotor.attach(rightMotorPin);
  leftMotor.attach(leftMotorPin);

  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encoder.attachHalfQuad(encoderPinA, encoderPinB); //A and B respectively
  encoder.clearCount();
}

void loop() {
  while(!done) {
   
    //Get values for 4 variables
    if(WiFi.status() == WL_CONNECTED) {
      currentState["cartPos"] = getCartPos();
      currentState["cartVel"] = getCartVel();
      currentState["poleAng"] = readEncoder();
      currentState["poleVel"] = getPoleVel();
          
      elapsedTime = millis();

      done = isDone();

      StaticJsonDocument<200> doc;   
      doc = addCurrentState(doc);
      doc = addPreviousState(doc);
      doc["action"] = action;
      doc["reward"] = reward;
      doc["done"] = done;

      if(done) {
        Serial.print("Ended with reward: ");
        Serial.println(reward);
        action = getPrediction(doc);
        rightMotor.writeMicroseconds(1500);
        leftMotor.writeMicroseconds(1500);
        break;
      }
      else {
        action = getPrediction(doc);
      }
    }

    //Go forward
    if(action == 1) {
      //Serial.println("Forward...");
      rightMotor.writeMicroseconds(1000);
      leftMotor.writeMicroseconds(2000);
      timeForward += ((millis()-elapsedTime)/1000);
    }
    //Go backward
    else {
      //Serial.println("Backward...");
      rightMotor.writeMicroseconds(2000);
      leftMotor.writeMicroseconds(1000);
      timeForward -= ((millis()-elapsedTime)/1000);
    }

    updatePreviousState();
  }

} //End of void loop()


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
  return (SPEED * timeForward);
}

double getCartVel() {
  Serial.println(millis()-elapsedTime);
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

boolean isDone() {
  if(currentState["cartPos"] > MAX_POS || currentState["cartPos"] < -MAX_POS || currentState["poleAng"] > MAX_ANG || currentState["poleAng"] < -MAX_ANG) {
    return true;
  }
  else {
    return false;
  }
}

