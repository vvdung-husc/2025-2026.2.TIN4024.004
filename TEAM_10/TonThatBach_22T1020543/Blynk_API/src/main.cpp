

#define BLYNK_TEMPLATE_ID "TMPL6GFBXls0K"
#define BLYNK_TEMPLATE_NAME "blynkApi"
#define BLYNK_AUTH_TOKEN "Q3Pn9x5UmisZAojRCMz31mS4UmxOPRQk"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define OPENWEATHERMAP_KEY "d16b55039d1462fde46c5dd50db2f5b0"

// dùng IP trực tiếp tránh lỗi DNS
#define WEATHER_HOST "http://147.75.40.76"

struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;

unsigned long currentMiliseconds = 0;
String urlWeather;

bool IsReady(unsigned long &timer, uint32_t ms) {
  if (currentMiliseconds - timer < ms) return false;
  timer = currentMiliseconds;
  return true;
}

String StringFormat(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  return String(buffer);
}

void getAPI() {

  ip4Info.ip4 = WiFi.localIP().toString();

  // tọa độ Huế
  ip4Info.latitude = "16.4637";
  ip4Info.longtitude = "107.5909";

  String map = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(),
    ip4Info.longtitude.c_str()
  );

  Serial.println("IP:");
  Serial.println(ip4Info.ip4);

  Serial.println("Google Maps:");
  Serial.println(map);

  urlWeather =
"https://188.166.183.43/data/2.5/weather?lat=16.4637&lon=107.5909&appid=" +
String(OPENWEATHERMAP_KEY) +
"&units=metric";

  Serial.println("Weather URL:");
  Serial.println(urlWeather);
}

void updateTemp() {

  static unsigned long lastTime = 0;

  if (millis() - lastTime >= 10000) {

    lastTime = millis();

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected");
      return;
    }

    WiFiClient client;
    HTTPClient http;

    Serial.println("Sending request...");

    http.begin(client,
      "http://api.openweathermap.org/data/2.5/weather?lat=16.4637&lon=107.5909&appid=d16b55039d1462fde46c5dd50db2f5b0&units=metric"
    );

    int httpCode = http.GET();

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {

      String payload = http.getString();
      Serial.println(payload);

      StaticJsonDocument<1024> doc;
      deserializeJson(doc, payload);

      float temp = doc["main"]["temp"];

      Serial.print("Temperature: ");
      Serial.println(temp);

      Blynk.virtualWrite(V3, temp);
    }

    http.end();
  }
}


void onceCalled() {

  static bool done = false;
  if (done) return;
  done = true;

  String link = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(),
    ip4Info.longtitude.c_str()
  );

  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, link);
}

void uptimeBlynk() {

  static unsigned long lastTime = 0;

  if (millis() - lastTime < 1000) return;

  lastTime = millis();

  unsigned long uptime = millis() / 1000;

  Blynk.virtualWrite(V0, uptime);
}

void setup() {

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");

  Blynk.config(BLYNK_AUTH_TOKEN, "128.199.144.129", 80);
  Blynk.connect();

  getAPI();
}

void loop() {

  Blynk.run();

  currentMiliseconds = millis();

  onceCalled();
  updateTemp();
  uptimeBlynk();
}