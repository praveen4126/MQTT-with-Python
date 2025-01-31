#define BLYNK_TEMPLATE_ID "TMPL3u3AoSzCt"
#define BLYNK_TEMPLATE_NAME "water"
#define BLYNK_AUTH_TOKEN "DwKYw7DvYGujdzRz9At0GVT-KS_gPe-u"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Pin Definitions
#define TRIGGER_PIN 32
#define ECHO_PIN 33
#define RED_LED 17
#define GREEN_LED 16
#define FAN_PIN 21

// Blynk Credentials
char auth[] = "DwKYw7DvYGujdzRz9At0GVT-KS_gPe-u";  // Replace with your Blynk Auth Token
char ssid[] = "HUAWEI_C331";     // Replace with your WiFi SSID
char pass[] = "12345678";        // Replace with your WiFi Password

// Variables
long duration;
int distance;
bool fanState = false;

// Fan control using Blynk Virtual Pin V1
BLYNK_WRITE(V1) {
  int value = param.asInt(); // 0 or 1
  fanState = value;

  if (fanState) {
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    Serial.println("Fan turned ON via Blynk");
  } else {
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(GREEN_LED, LOW);
    Serial.println("Fan turned OFF via Blynk");
  }
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  // Pin modes
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  // Initialize pins
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(FAN_PIN, LOW);

  Serial.println("Water Level Monitor Initialized");
}

void loop() {
  Blynk.run();
  checkWaterLevel();
}

// Function to measure water level
void checkWaterLevel() {
  // Trigger ultrasonic sensor
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Measure the duration of the echo pulse
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance (in cm)
  distance = duration * 0.034 / 2;

  // Send water level to Blynk app (Virtual Pin V2)
  Blynk.virtualWrite(V2, distance);

  // Print water level to Serial Monitor
  Serial.print("Water Level: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Check if water level is critical and control red LED
  if (distance <= 10 || distance >= 100) { // Adjust thresholds as needed
    digitalWrite(RED_LED, HIGH);
    Serial.println("Warning: Water level critical!");

    // Notify the user via Blynk (requires event setup in Blynk dashboard)
    Blynk.logEvent("water_level_critical", "Water level critical!");
  } else {
    digitalWrite(RED_LED, LOW);
  }

  delay(1000); // Delay for better readability in the Serial Monitor
}
