#define BLYNK_TEMPLATE_ID "TMPL65DqTYs6o"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "G6RidV6rr54U7oiL6M-0wZKhSglPNDx2"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

BlynkTimer timer;

String apiKey = "f776a4341c39f68ae84c5f5dfcdc9f09";

String ipv4 = "";

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
// LẤY IPv4
void getIP() {

  HTTPClient http;

  http.begin("http://api.ipify.org");

  int code = http.GET();

  if (code == 200) {

    ipv4 = http.getString();

    Serial.print("IPv4: ");
    Serial.println(ipv4);

    Blynk.virtualWrite(V1, ipv4);
  }

  http.end();
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

    Blynk.virtualWrite(V3, temp);

  } else {

    Serial.println("Weather API error");

  }

  http.end();
}

//////////////////////////////////////////////////////
// GỬI LINK GOOGLE MAP
void sendMap() {

  String mapLink =
  "https://www.google.com/maps?q=" +
  String(lat,6) + "," + String(lon,6);

  Serial.print("Map: ");
  Serial.println(mapLink);

  Blynk.virtualWrite(V2, mapLink);
}

//////////////////////////////////////////////////////
// SETUP
void setup() {

  Serial.begin(115200);

  Serial.println("Connecting WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("WiFi Connected");

  getIP();        // lấy IPv4
  sendMap();      // gửi Google Map

  timer.setInterval(1000L, sendUptime);
  timer.setInterval(15000L, getWeather);
}

//////////////////////////////////////////////////////
// LOOP
void loop() {

  Blynk.run();
  timer.run();
}