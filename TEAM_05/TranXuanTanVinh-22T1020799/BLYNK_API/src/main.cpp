#define BLYNK_TEMPLATE_ID "TMPL6HqD1EZwB"
#define BLYNK_TEMPLATE_NAME "esp32api"
#define BLYNK_AUTH_TOKEN "5_1_Rk6x7MQ0fXxtPliGk8rzXqFn4KSr"



#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

BlynkTimer timer;

String ipv4 = "";
float lat = 0;
float lon = 0;
float temperature = 0;

String apiKey = "824f64ca64a0da3b0db9b90827e7c5d4";

unsigned long uptime = 0;

//////////////////////////////////////////////////////
// ===== Thời gian hoạt động =====
void sendUptime() {

  uptime++;

  Serial.print("Uptime: ");
  Serial.println(uptime);

  Blynk.virtualWrite(V0, uptime);
}

//////////////////////////////////////////////////////
// ===== Lấy IPv4 + vị trí =====
void getLocation() {

  HTTPClient http;
  http.begin("http://ip-api.com/json");

  int httpCode = http.GET();

  if (httpCode == 200) {

    String payload = http.getString();

    Serial.println("Location API Response:");
    Serial.println(payload);

    StaticJsonDocument<1024> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {

      ipv4 = doc["query"].as<String>();
      lat = doc["lat"];
      lon = doc["lon"];

      Serial.print("IPv4: ");
      Serial.println(ipv4);

      String googleMap =
      "https://www.google.com/maps?q=" +
      String(lat,6) + "," + String(lon,6);

      Serial.println("Google Maps:");
      Serial.println(googleMap);

      Blynk.virtualWrite(V1, ipv4);
      Blynk.virtualWrite(V2, googleMap);
    }
    else {
      Serial.println("JSON Location Error");
    }
  }
  else {
    Serial.println("Location API Error");
  }

  http.end();
}

//////////////////////////////////////////////////////
// ===== Lấy nhiệt độ =====
void getWeather() {

  if(lat == 0 || lon == 0) return;

  HTTPClient http;

  String url =
  "http://api.openweathermap.org/data/2.5/weather?lat=" +
  String(lat) +
  "&lon=" + String(lon) +
  "&appid=" + apiKey +
  "&units=metric";

  http.begin(url);

  int httpCode = http.GET();

  if (httpCode == 200) {

    String payload = http.getString();

    Serial.println("Weather API Response:");
    Serial.println(payload);

    StaticJsonDocument<2048> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {

      temperature = doc["main"]["temp"];

      Serial.print("Temperature: ");
      Serial.println(temperature);

      Blynk.virtualWrite(V3, temperature);
    }
    else {
      Serial.println("JSON Weather Error");
    }
  }
  else {
    Serial.println("Weather API Error");
  }

  http.end();
}

//////////////////////////////////////////////////////
// ===== Setup =====
void setup() {

  Serial.begin(115200);

  Serial.println("Connecting WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Connected to Blynk");

  timer.setInterval(1000L, sendUptime);
  timer.setInterval(10000L, getLocation);
  timer.setInterval(15000L, getWeather);
}

//////////////////////////////////////////////////////
// ===== Loop =====
void loop() {

  Blynk.run();
  timer.run();
}
