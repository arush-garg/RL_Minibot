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

const double SPEED = 0.144; //In meters per second

const double MAX_POS = 2.4; //In meters
const double MAX_ANG = 0.418; //In radians (24 degrees) - Original Gymnasium environment ends at 0.2095, but a greater angle is acceptable for this project


// State variables as dictionaries
unordered_map<string, float> currentState;
unordered_map<string, float> previousState;

int action = -1;
int reward = 0;
bool done = true; //Start when button is pressed


long elapsedTime;
double timeForward = 0;

int rightMotorPin = 14;
int leftMotorPin = 13;

int encoderPinA = 18;
int encoderPinB = 5; //CHECK AND UPDATE

int buttonPin = 21;

Servo rightMotor;
Servo leftMotor;

ESP32Encoder encoder;


JsonDocument addCurrentState(JsonDocument doc) {
  JsonArray current_state = doc.createNestedArray("current_state");
  current_state.add(currentState["cartPos"]);
  current_state.add(currentState["cartVel"]);
  current_state.add(currentState["poleAng"]);
  current_state.add(currentState["poleVel"]); 
  return doc;  
}

JsonDocument addPreviousState(JsonDocument doc) {
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

  pinMode(buttonPin, INPUT_PULLUP);

  rightMotor.attach(rightMotorPin);
  leftMotor.attach(leftMotorPin);

  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encoder.attachHalfQuad(encoderPinA, encoderPinB);
  encoder.clearCount();
}

void loop() {

  if(digitalRead(buttonPin) == LOW) {
    reset();
  }

  while(!done) {
   
    //Get values for 4 variables
    if(WiFi.status() == WL_CONNECTED) {
      currentState["cartPos"] = getCartPos();
      currentState["cartVel"] = getCartVel();
      currentState["poleAng"] = readEncoder();
      currentState["poleVel"] = getPoleVel();
          
      elapsedTime = millis();

      done = isDone();

      JsonDocument doc;   
      doc = addCurrentState(doc);
      doc = addPreviousState(doc);
      doc["action"] = action;
      doc["reward"] = reward;
      doc["done"] = done;

      printJsonDoc(doc);

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
        reward++;
      }
    }

    //Go backward
    if(action == 0) {
      rightMotor.writeMicroseconds(1000);
      leftMotor.writeMicroseconds(2000);
      timeForward -= (((double)(millis()-elapsedTime))/1000);
    }
    //Go forward
    else if (action == 1) {
      rightMotor.writeMicroseconds(2000);
      leftMotor.writeMicroseconds(1000);
      timeForward += (((double)(millis()-elapsedTime))/1000);
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
  double cartVel =  1000*(currentState["cartPos"]-previousState["cartPos"])/(millis()-elapsedTime);

  if(isnan(cartVel)) {
    return 0.0;
  }
  else {
    return cartVel;
  }
}

double getPoleVel() {
  double poleVel = 1000*(currentState["poleAng"]-previousState["poleAng"])/(millis()-elapsedTime);
  if(isnan(poleVel)) {
    return 0.0;
  }
  else {
    return poleVel;
  }
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

bool isDone() {
  if(currentState["cartPos"] > MAX_POS || currentState["cartPos"] < -MAX_POS || currentState["poleAng"] > MAX_ANG || currentState["poleAng"] < -MAX_ANG) {
    return true;
  }
  else {
    return false;
  }
}

void printJsonDoc(JsonDocument doc) {
  Serial.printf("Pos: %f  Vel: %f PoleAng: %f PoleVel: %f \n", doc["cartPos"], doc["cartVel"], doc["poleAng"], doc["poleVel"]);
}

void reset() {
  done = false;
  encoder.clearCount();
  timeForward = 0;
  reward = 0;
}