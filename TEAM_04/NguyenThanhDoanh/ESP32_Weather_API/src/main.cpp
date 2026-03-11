#include <Arduino.h>

// Thông tin thiết bị Blynk của bạn
#define BLYNK_TEMPLATE_ID "TMPL6QXkSWuB5"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "ygNGyXeU3Z5tSxOWNVFJK_nIqgKoLvMG"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define OPENWEATHERMAP_KEY "eb912a2c2355036a5da46c9d22fb52d5"

struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;
String urlWeather = "";

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (millis() - ulTimer < milisecond) return false;
  ulTimer = millis();
  return true;
}

// SỬA LỖI: Hàm phân tách chuỗi chuẩn xác hơn
void parseGeoInfo(String payload, IP4_Info &ipInfo) {
  String values[7];
  int index = 0;
  int lastDelimiter = 0;

  for (int i = 0; i < payload.length() && index < 6; i++) {
    if (payload[i] == '|') {
      values[index++] = payload.substring(lastDelimiter, i);
      lastDelimiter = i + 1;
    }
  }
  values[6] = payload.substring(lastDelimiter); // Lấy phần Latitude cuối cùng

  ipInfo.ip4 = values[0];
  ipInfo.longtitude = values[5]; // Kinh độ
  ipInfo.latitude = values[6];   // Vĩ độ

  Serial.println("--- TOA DO DA LAY ---");
  Serial.printf("Lat: %s, Lon: %s\n", ipInfo.latitude.c_str(), ipInfo.longtitude.c_str());
}

void getGeoAPI() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://ip4.iothings.vn/?geo=1");
    if (http.GET() > 0) {
      String response = http.getString();
      parseGeoInfo(response, ip4Info);

      // Tạo URL thời tiết
      urlWeather = "https://api.openweathermap.org/data/2.5/weather?lat=" + ip4Info.latitude +
                   "&lon=" + ip4Info.longtitude + "&appid=" + String(OPENWEATHERMAP_KEY) + "&units=metric";
    }
    http.end();
  }
}

void updateWeather() {
  static unsigned long lastTempTime = 0;
  if (!IsReady(lastTempTime, 15000)) return; // Cập nhật mỗi 15 giây

  if (WiFi.status() == WL_CONNECTED && urlWeather.length() > 0) {
    HTTPClient http;
    http.begin(urlWeather);
    int httpCode = http.GET();

    if (httpCode == 200) {
      JsonDocument doc;
      deserializeJson(doc, http.getString());
      float temp = doc["main"]["temp"];
      
      Serial.printf("Nhiet do thuc te: %.2f C\n", temp);
      Blynk.virtualWrite(V3, temp); // Đảm bảo Datastream V3 là Gauge
    } else {
      Serial.printf("Loi API Thoi tiet: %d\n", httpCode);
    }
    http.end();
  }
}

// Các hàm updateUptime và sendStaticInfo giữ nguyên như code của bạn
void sendStaticInfo() {
  static bool sent = false;
  if (sent || ip4Info.ip4.length() == 0) return;
  String mapsLink = "https://www.google.com/maps?q=" + ip4Info.latitude + "," + ip4Info.longtitude;
  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, mapsLink);
  sent = true;
}

void updateUptime() {
  static unsigned long lastUptime = 0;
  if (IsReady(lastUptime, 1000)) Blynk.virtualWrite(V0, millis() / 1000);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  getGeoAPI();
}

void loop() {
  Blynk.run();
  updateUptime();
  sendStaticInfo();
  updateWeather();
}