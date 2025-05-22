#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


//const char* serverAddress = "http://10.0.0.163:8000/predict";
const char* serverAddress = "http://172.20.10.4:8000/predict";

int getPrediction(JsonDocument doc) {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverAddress);

  http.addHeader("Content-Type", "application/json");

  // Serialize JSON object to string
  String jsonData;
  serializeJson(doc, jsonData);

  // Send the POST request with the JSON data
  int httpResponseCode = http.POST(jsonData);
  String response = http.getString();
  JsonDocument docResponse;
  DeserializationError error = deserializeJson(docResponse, response);

  if (error) {
    Serial.println("Failed to parse JSON response");
    http.end();
    return -1;
  } 
  else {
    // Extract the result from the JSON response
    int action = (int)docResponse["action"];
    http.end();
    return action;
  }
}