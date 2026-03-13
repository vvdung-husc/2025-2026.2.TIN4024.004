#define BLYNK_TEMPLATE_ID "TMPL6FP6o8Exj"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "Nlea05kEKJcCLA85VK8t1DAIAYLQIKoM"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

String ip;
float lat;
float lon;
float temp;

String weatherKey = "29975be309f32b6598d4fcb2883ae0bb";

BlynkTimer timer;

void getLocation()
{
  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();

    Serial.println(payload);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    ip = doc["ip"].as<String>();
    lat = doc["latitude"];
    lon = doc["longitude"];

    Serial.println("IPv4: " + ip);

    String mapLink = "https://www.google.com/maps/place/" +
                     String(lat,6) + "," + String(lon,6);

    Serial.println("Google Maps:");
    Serial.println(mapLink);

    Blynk.virtualWrite(V1, ip);
    Blynk.virtualWrite(V2, mapLink);
  }

  http.end();
}

void getWeather()
{
  HTTPClient http;

  String url =
  "https://api.openweathermap.org/data/2.5/weather?lat="
  + String(lat) +
  "&lon=" +
  String(lon) +
  "&units=metric&appid=" +
  weatherKey;

  http.begin(url);

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    temp = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    Blynk.virtualWrite(V3, temp);
  }

  http.end();
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(10000L, getLocation);
  timer.setInterval(15000L, getWeather);
}

void loop()
{
  Blynk.run();
  timer.run();
}