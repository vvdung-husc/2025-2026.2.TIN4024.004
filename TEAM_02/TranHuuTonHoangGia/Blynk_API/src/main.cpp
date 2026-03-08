#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6fvtn-tDn"
#define BLYNK_TEMPLATE_NAME "BLYNK API"
#define BLYNK_AUTH_TOKEN "m3Q8wBe0WCtgNqHR4IQFyBN7PSSpyQeL"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define OPENWEATHERMAP_KEY "6470fea3e9213cde2af02e2d2530ab46"

String ipv4 = "";
String latitude = "";
String longitude = "";
String weatherURL = "";

unsigned long lastWeather = 0;

void getIPLocation()
{
  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println(payload);

    String data[7];
    int index = 0;

    while (payload.length() > 0 && index < 7)
    {
      int pos = payload.indexOf('|');

      if (pos == -1)
      {
        data[index++] = payload;
        break;
      }

      data[index++] = payload.substring(0, pos);
      payload = payload.substring(pos + 1);
    }

    ipv4 = data[0];
    longitude = data[5];
    latitude = data[6];

    Serial.println(ipv4);
    Serial.println(latitude);
    Serial.println(longitude);

    weatherURL =
        "https://api.openweathermap.org/data/2.5/weather?lat=16.27&lon=107.35&appid=6470fea3e9213cde2af02e2d2530ab46" +
        latitude +
        "&lon=" +
        longitude +
        "&appid=" +
        OPENWEATHERMAP_KEY +
        "&units=metric";
  }

  http.end();
}

void sendLocationToBlynk()
{
  String mapLink =
      "https://www.google.com/maps/place/" +
      latitude +
      "," +
      longitude;

  Blynk.virtualWrite(V5, ipv4);
  Blynk.virtualWrite(V6, mapLink);
}

void getWeather()
{
  if (millis() - lastWeather < 10000)
    return;

  lastWeather = millis();

  if (weatherURL == "")
    return;

  HTTPClient http;

  http.begin(weatherURL);

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String payload = http.getString();

    Serial.println(payload);

    JsonDocument doc;

    deserializeJson(doc, payload);

    float temp = doc["main"]["temp"];

    Serial.print("Temp: ");
    Serial.println(temp);

    Blynk.virtualWrite(V2, temp);
  }

  http.end();
}

void sendUptime()
{
  static unsigned long last = 0;

  if (millis() - last < 1000)
    return;

  last = millis();

  Blynk.virtualWrite(V0, millis() / 1000);
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }

  Serial.println("Connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  getIPLocation();

  sendLocationToBlynk();
}

void loop()
{
  Blynk.run();

  sendUptime();

  getWeather();
}