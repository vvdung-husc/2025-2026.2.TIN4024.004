/*************** BLYNK CONFIG *****************/

#define WIFI_SSID "your_wifi"
#define WIFI_PASS "your_pass"
#define OPENWEATHER_API_KEY "API_THAT_IS_SECRET"

#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/*************** WIFI *****************/
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

/*************** OPEN WEATHER ***************/
#define OPENWEATHERMAP_KEY ""

#define LATITUDE  "16.4637"
#define LONGITUDE "107.5909"

/*************** STRUCT *****************/
struct IP4_Info {
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;

unsigned long currentMillis;

/*************** FORMAT STRING *****************/
String StringFormat(const char* fmt, ...) {
  char buffer[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  return String(buffer);
}

/*************** GET INFO *****************/
void getAPI() {

  ip4Info.ip4 = WiFi.localIP().toString();
  ip4Info.latitude = LATITUDE;
  ip4Info.longitude = LONGITUDE;

  String mapLink = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    LATITUDE, LONGITUDE
  );

  Serial.println("IP: " + ip4Info.ip4);
  Serial.println("Map: " + mapLink);
}

/*************** WEATHER UPDATE *****************/
void updateTemp() {

  static unsigned long lastTime = 0;
  if (millis() - lastTime < 10000) return;
  lastTime = millis();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    return;
  }

  HTTPClient http;
  WiFiClient client;

  String url =
    "http://api.openweathermap.org/data/2.5/weather?lat=" +
    String(LATITUDE) +
    "&lon=" +
    String(LONGITUDE) +
    "&appid=" +
    OPENWEATHERMAP_KEY +
    "&units=metric";

  Serial.println("Requesting weather...");

  http.begin(client, url);

  int httpCode = http.GET();

  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  if (httpCode == HTTP_CODE_OK) {

    String payload = http.getString();

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {

      float temp = doc["main"]["temp"];

      Serial.print("Temperature: ");
      Serial.println(temp);

      Blynk.virtualWrite(V3, temp);
    }
  }

  http.end();
}

/*************** SEND INFO ONCE *****************/
void sendDeviceInfo() {

  static bool sent = false;
  if (sent || !Blynk.connected()) return;

  sent = true;

  String mapLink = StringFormat(
    "https://www.google.com/maps/place/%s,%s",
    LATITUDE, LONGITUDE
  );

  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, mapLink);
}

/*************** UPTIME *****************/
void uptimeBlynk() {

  static unsigned long lastTime = 0;
  if (millis() - lastTime < 1000) return;
  lastTime = millis();

  Blynk.virtualWrite(V0, millis() / 1000);
}

/*************** SETUP *****************/
void setup() {

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  Blynk.config(BLYNK_AUTH_TOKEN, "128.199.144.129", 80);
  Blynk.connect();

  getAPI();
}

/*************** LOOP *****************/
void loop() {

  Blynk.run();

  currentMillis = millis();

  sendDeviceInfo();
  updateTemp();
  uptimeBlynk();
}