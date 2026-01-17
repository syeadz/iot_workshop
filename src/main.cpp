#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HCSR04.h>

// Driver for ESP32 USB to Serial converters (CP210x)
// Install drivers if needed:
// https://randomnerdtutorials.com/install-esp32-esp8266-usb-drivers-cp210x-windows/
// https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads

// =========================
// SECTION 0 – CONFIG
// =========================

// TODO: Enter your WiFi credentials
const char* WIFI_SSID = "???";
const char* WIFI_PASS = "???!";

// TODO: Enter your Flask server URL (use local network IP)
const char* SERVER_URL = "http://<ip>/api/update";

// TODO: Assign a unique device ID
const char* DEVICE_ID = "esp32_1";

// =========================
// SECTION 1 – PINS
// =========================
#define LED_PIN 2
#define TRIGGER_PIN 23
#define ECHO_PIN 18

UltraSonicDistanceSensor distanceSensor(TRIGGER_PIN, ECHO_PIN);

// =========================
// SECTION 2 – VARIABLES
// =========================
double detectionThreshold = 50.0; // default threshold

// =========================
// FUNCTION 1 – CONNECT WIFI
// =========================
void connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
    Serial.print("IP Address: "); Serial.println(WiFi.localIP());
}

// =========================
// FUNCTION 2 – READ DISTANCE
// =========================
double readDistance() {
    // TODO: Add sensor reading
    double distance = distanceSensor.measureDistanceCm();
    if (distance < 0) {
        Serial.println("Invalid distance reading!");
    } else {
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
    }
    return distance;
}

// =========================
// FUNCTION 3 – CONTROL LED
// =========================
void updateLED(double distance) {
    if (distance < detectionThreshold) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
}

// =========================
// FUNCTION 4 – SEND DATA TO SERVER
// =========================
void sendDataToServer(double distance) {
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    String json = "{";
    json += "\"id\":\"" + String(DEVICE_ID) + "\",";
    json += "\"distance\":" + String(distance, 2);
    json += "}";

    int httpCode = http.POST(json);

    if (httpCode == 200) {
        String payload = http.getString();
        Serial.println("Server response: " + payload);

        int tIndex = payload.indexOf("threshold");
        if (tIndex != -1) {
            double newThreshold = payload.substring(payload.indexOf(':', tIndex) + 1).toDouble();
            detectionThreshold = newThreshold;
            Serial.print("Updated threshold from server: ");
            Serial.println(detectionThreshold);
        }
    } else {
        Serial.print("HTTP error: "); Serial.println(httpCode);
    }

    http.end();
}

// =========================
// SETUP
// =========================
void setup() {
    Serial.begin(9600);
    pinMode(LED_PIN, OUTPUT);
    connectWiFi();
    Serial.println("Setup complete!");
}

// =========================
// LOOP
// =========================
void loop() {
    double distance = readDistance();    // Task 1: Read sensor
    updateLED(distance);                 // Task 2: Control LED
    sendDataToServer(distance);          // Task 3: Send to server and update threshold
    delay(500);                          // pause between readings
}
