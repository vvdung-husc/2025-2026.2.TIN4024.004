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

// Tọa độ Huế
float lat = 16.4591267;
float lon = 107.5901477;

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
// LẤY NHIỆT ĐỘ
void getWeather() {

  HTTPClient http;

  String url =
  "http://api.openweathermap.org/data/2.5/weather?lat=" +
  String(lat,6) +
  "&lon=" + String(lon,6) +
  "&appid=" + apiKey +
  "&units=metric";

  Serial.println("Request:");
  Serial.println(url);

  http.begin(url);

  int code = http.GET();

  Serial.print("HTTP Code: ");
  Serial.println(code);

  if (code == 200) {

    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    float temp = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    Blynk.virtualWrite(V2, temp);

  } else {

    Serial.println("Failed to get weather data");

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

  timer.setInterval(1000L, sendUptime);
  timer.setInterval(15000L, getWeather);
}

//////////////////////////////////////////////////////
// LOOP
void loop() {

  Blynk.run();
  timer.run();
}