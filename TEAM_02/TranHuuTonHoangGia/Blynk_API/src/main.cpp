#define BLYNK_TEMPLATE_ID "TMPL6fvtn-tDn"
#define BLYNK_TEMPLATE_NAME "BLYNK API"
#define BLYNK_AUTH_TOKEN "m3Q8wBe0WCtgNqHR4IQFyBN7PSSpyQeL"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

BlynkTimer timer;

String apiKey = "6470fea3e9213cde2af02e2d2530ab46";

String ipv4 = "";
float lat = 0;
float lon = 0;

unsigned long uptime = 0;

//////////////////////////////////////////////////////
// UPTIME
void sendUptime() {

  uptime++;

  Serial.print("Uptime: ");
  Serial.println(uptime);

  Blynk.virtualWrite(V0, uptime);
}

//////////////////////////////////////////////////////
// LẤY VỊ TRÍ (1 LẦN)
void getLocation() {

  HTTPClient http;
  http.begin("http://ip-api.com/json");

  int code = http.GET();

  if (code == 200) {

    String payload = http.getString();

    Serial.println("Location Data:");
    Serial.println(payload);

    JsonDocument doc;

    deserializeJson(doc, payload);

    ipv4 = doc["query"].as<String>();
    lat = doc["lat"];
    lon = doc["lon"];

    Serial.print("IP: ");
    Serial.println(ipv4);

    String mapLink =
    "https://www.google.com/maps?q=" +
    String(lat,6) + "," + String(lon,6);

    Blynk.virtualWrite(V5, ipv4);
    Blynk.virtualWrite(V6, mapLink);
  }

  http.end();
}

//////////////////////////////////////////////////////
// LẤY NHIỆT ĐỘ
void getWeather() {

  if (lat == 0 || lon == 0) return;

  HTTPClient http;

  String url =
  "http://api.openweathermap.org/data/2.5/weather?lat=" +
  String(lat) +
  "&lon=" + String(lon) +
  "&appid=" + apiKey +
  "&units=metric";

  http.begin(url);

  int code = http.GET();

  if (code == 200) {

    String payload = http.getString();

    Serial.println("Weather Data:");
    Serial.println(payload);

    JsonDocument doc;

    deserializeJson(doc, payload);

    float temp = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    Blynk.virtualWrite(V2, temp);
  }

  http.end();
}

//////////////////////////////////////////////////////
// SETUP
void setup() {

  Serial.begin(115200);

  Serial.println("Connecting WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("WiFi Connected");

  getLocation();   // lấy vị trí 1 lần

  timer.setInterval(1000L, sendUptime);
  timer.setInterval(15000L, getWeather);
}

//////////////////////////////////////////////////////
// LOOP
void loop() {

  Blynk.run();
  timer.run();
}