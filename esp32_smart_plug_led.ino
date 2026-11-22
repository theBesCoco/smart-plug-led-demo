/*
  Smart Plug LED Test - ESP32
  LED ON when server returns "1"; LED OFF when server returns "0".
  Use a LOW-VOLTAGE LED only. Do not connect mains voltage.
*/

#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Replace with your deployed Render URL, without a trailing slash.
const char* SERVER_URL = "https://YOUR-SERVICE.onrender.com/api/esp";
const char* DEVICE_KEY = "change-this-device-key";

const int LED_PIN = 2;          // External LED GPIO; change to a suitable GPIO if needed
const int POLL_INTERVAL_MS = 2000;

unsigned long lastPoll = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() - lastPoll >= POLL_INTERVAL_MS) {
    lastPoll = millis();

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(SERVER_URL);
      http.addHeader("x-device-key", DEVICE_KEY);
      int code = http.GET();

      if (code == HTTP_CODE_OK) {
        String command = http.getString();
        command.trim();

        if (command == "1") {
          digitalWrite(LED_PIN, HIGH);
          Serial.println("LED ON");
        } else if (command == "0") {
          digitalWrite(LED_PIN, LOW);
          Serial.println("LED OFF");
        }
      } else {
        Serial.printf("Server error: %d\n", code);
        // Fail safe: turn LED off if the server cannot be reached.
        digitalWrite(LED_PIN, LOW);
      }
      http.end();
    } else {
      digitalWrite(LED_PIN, LOW);
      Serial.println("Wi-Fi disconnected");
    }
  }
}
