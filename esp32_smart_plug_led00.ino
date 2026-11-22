
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// แก้ข้อมูลให้ตรงกับของคุณ
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* SERVER_URL = "https://YOUR-APP.onrender.com/api/esp";
const char* DEVICE_KEY = "YOUR_DEVICE_KEY";

// ขา Relay
const int relayPins[4] = {23, 22, 21, 19};

// รีเลย์ส่วนใหญ่เป็น Active LOW
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

unsigned long lastCheck = 0;

void setRelay(int channel, bool on) {
  digitalWrite(
    relayPins[channel],
    on ? RELAY_ON : RELAY_OFF
  );
}

void setup() {
  Serial.begin(115200);

  // ตั้ง Relay ทุกช่องเป็น OFF ก่อน
  for (int i = 0; i < 4; i++) {
    digitalWrite(relayPins[i], RELAY_OFF);
    pinMode(relayPins[i], OUTPUT);
    setRelay(i, false);
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(500);
    return;
  }

  if (millis() - lastCheck >= 2000) {
    lastCheck = millis();

    WiFiClientSecure client;
    client.setInsecure(); // ใช้ทดสอบ HTTPS

    HTTPClient http;

    http.begin(client, SERVER_URL);
    http.addHeader("x-device-key", DEVICE_KEY);

    int httpCode = http.GET();

    if (httpCode == 200) {
      String data = http.getString();
      data.trim();

      if (data.length() == 4 &&
          (data[0] == '0' || data[0] == '1') &&
          (data[1] == '0' || data[1] == '1') &&
          (data[2] == '0' || data[2] == '1') &&
          (data[3] == '0' || data[3] == '1')) {

        // ใช้เฉพาะ Relay ช่อง 1
        setRelay(0, data[0] == '1');

        // ปิด Relay ช่อง 2-4
        setRelay(1, false);
        setRelay(2, false);
        setRelay(3, false);

        Serial.print("Relay 1: ");
        Serial.println(data[0] == '1' ? "ON" : "OFF");
      }
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(httpCode);
    }

    http.end();
  }
}
