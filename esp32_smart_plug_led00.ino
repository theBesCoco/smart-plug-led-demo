/*
  Smart Plug LED Test - ESP32
  ESP32 เรียก GET /api/esp ทุก 2 วินาที แล้วสั่ง LED ตามค่าที่ได้ (1 = เปิด, 0 = ปิด)

  *** ก่อนใช้งาน แก้ 4 ค่าด้านล่างในเครื่องคุณเท่านั้น ***
  *** อย่าอัปโหลดรหัส Wi-Fi หรือ DEVICE_KEY จริงขึ้น GitHub (repo เป็น Public) ***
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ====== แก้ค่าตรงนี้ ======
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL    = "https://YOUR-SERVICE.onrender.com";  // ไม่ต้องมี / ท้ายสุด
const char* DEVICE_KEY    = "YOUR_DEVICE_KEY";                    // ต้องตรงกับ DEVICE_KEY ใน Render
// ==========================

const int LED_PIN = 2;                         // GPIO2 = LED บนบอร์ด ESP32 ส่วนใหญ่ (ถ้าต่อ LED ภายนอก ให้ใส่ตัวต้านทาน 220-330 โอห์ม)
const unsigned long POLL_INTERVAL_MS = 2000;   // เรียกทุก 2 วินาที
const int HTTP_TIMEOUT_MS = 20000;             // Render ฟรีตอนตื่นจากหลับอาจช้า 30-60 วินาที

unsigned long lastPoll = 0;
int failCount = 0;

void connectWiFi() {
  Serial.printf("เชื่อมต่อ Wi-Fi: %s ", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {  // รอสูงสุดประมาณ 20 วินาที
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nเชื่อมต่อสำเร็จ IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nเชื่อมต่อ Wi-Fi ไม่สำเร็จ จะลองใหม่");
  }
}

void setLed(bool on) {
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}

// แปลงข้อความที่ server ตอบกลับเป็น เปิด/ปิด
// รองรับทั้งข้อความ "1"/"0" และ JSON ที่มี "on":true / "isOn":true
bool parseOn(String body) {
  body.trim();
  if (body == "1") return true;
  if (body == "0") return false;
  if (body.indexOf("\"on\":true") >= 0 || body.indexOf("\"isOn\":true") >= 0 ||
      body.indexOf("\"on\": true") >= 0 || body.indexOf("\"isOn\": true") >= 0) {
    return true;
  }
  return false;
}

void pollServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi หลุด กำลังเชื่อมต่อใหม่...");
    connectWiFi();
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();  // ข้ามการตรวจใบรับรอง เหมาะกับการทดสอบ (ถ้าจะใช้จริงควรใส่ root CA)

  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);

  String url = String(SERVER_URL) + "/api/esp";
  if (!http.begin(client, url)) {
    Serial.println("http.begin ล้มเหลว");
    return;
  }
  http.addHeader("x-device-key", DEVICE_KEY);

  int code = http.GET();

  if (code == 200) {
    String body = http.getString();
    bool on = parseOn(body);
    setLed(on);
    failCount = 0;
    Serial.printf("[OK] ตอบกลับ: %s -> LED %s\n", body.c_str(), on ? "ON" : "OFF");
  } else if (code == 401 || code == 403) {
    Serial.printf("[ERR %d] DEVICE_KEY ไม่ตรงกับที่ตั้งไว้ใน Render\n", code);
    failCount++;
  } else {
    Serial.printf("[ERR %d] %s\n", code, http.errorToString(code).c_str());
    failCount++;
  }

  http.end();

  // ถ้าล้มเหลวติดกันหลายครั้ง ให้ปิด LED เพื่อความปลอดภัย
  if (failCount >= 5) {
    setLed(false);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED_PIN, OUTPUT);
  setLed(false);

  Serial.println("\n=== Smart Plug LED Test ===");
  connectWiFi();
}

void loop() {
  unsigned long now = millis();
  if (now - lastPoll >= POLL_INTERVAL_MS) {
    lastPoll = now;
    pollServer();
  }
}
