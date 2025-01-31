#include <WiFi.h>
#include <PubSubClient.h>
#include <NewPing.h>

// WiFi credentials
const char* ssid = "HUAWEI_C47D";
const char* password = "12345678";

// MQTT broker details
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

// MQTT topics
const char* waterLevelTopic = "water_level";
const char* greenLedControlTopic = "green_led_control";

// Pin definitions
#define TRIGGER_PIN 32
#define ECHO_PIN 33
#define RED_LED_PIN 17
#define GREEN_LED_PIN 16

// HC-SR04 settings
#define MAX_DISTANCE 5 // Tank height in cm
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Water level thresholds
const int minWaterLevel = 0;  
const int maxWaterLevel = 5; 

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  connectWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  connectMQTT();
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  static unsigned long lastMillis = 0;
  if (millis() - lastMillis >= 2000) { 
    lastMillis = millis();
    
    int waterLevel = measureWaterLevel();
    Serial.print("Water Level: ");
    Serial.print(waterLevel);
    Serial.println(" cm");

    publishWaterLevel(waterLevel);
    controlRedLED(waterLevel);
  }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(greenLedControlTopic);
    } else {
      Serial.println(" Failed. Retrying in 5s");
      delay(5000);
    }
  }
}

void reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();  
  }
  connectMQTT();
}

int measureWaterLevel() {
  int distance = sonar.ping_cm();
  int waterLevel = MAX_DISTANCE - distance;  // Convert distance to water level
  if (waterLevel < 0) waterLevel = 0;  // Avoid negative values
  if (waterLevel > MAX_DISTANCE) waterLevel = MAX_DISTANCE;  // Avoid values beyond max height
  return waterLevel;
}

void publishWaterLevel(int level) {
  char waterLevelStr[10];
  dtostrf(level, 4, 2, waterLevelStr);
  if (client.connected()) {
    client.publish(waterLevelTopic, waterLevelStr);
  }
}

void controlRedLED(int level) {
  digitalWrite(RED_LED_PIN, (level <= minWaterLevel || level >= maxWaterLevel) ? HIGH : LOW);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == greenLedControlTopic) {
    digitalWrite(GREEN_LED_PIN, message == "ON" ? HIGH : LOW);
  }
}
