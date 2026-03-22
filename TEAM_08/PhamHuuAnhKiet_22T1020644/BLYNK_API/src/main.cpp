#define BLYNK_TEMPLATE_ID "TMPL6maKop6k2"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "MxLagMP0qVpW0A565pPmU8jsoxywHO2f"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "YOUR_WIFI";
char pass[] = "YOUR_PASSWORD";

String apiKey = "5a5c4a9fec1b2237889827fd747858d6";

float lat;
float lon;

BlynkTimer timer;

void getIPLocation()
{
  HTTPClient http;

  http.begin("http://ip4.iothings.vn?geo=1");
  int httpCode = http.GET();

  if (httpCode == 200)
  {
    String payload = http.getString();

    Serial.println("API Response:");
    Serial.println(payload);

    // Tách chuỗi
    int p1 = payload.indexOf('|');
    int p6 = payload.lastIndexOf('|');

    String ip = payload.substring(0, p1);
    String latStr = payload.substring(p6 + 1);

    int p5 = payload.lastIndexOf('|', p6 - 1);
    String lonStr = payload.substring(p5 + 1, p6);

    lat = latStr.toFloat();
    lon = lonStr.toFloat();

    String googleMap =
      "https://www.google.com/maps/place/" +
      String(lat,6) + "," + String(lon,6);

    Serial.println("IP:");
    Serial.println(ip);

    Serial.println("Google Maps:");
    Serial.println(googleMap);

    Blynk.virtualWrite(V1, ip);
    Blynk.virtualWrite(V2, googleMap);
  }

  http.end();
}

void getWeather()
{
  HTTPClient http;

  String url =
  "https://api.openweathermap.org/data/2.5/weather?lat=" +
  String(lat,6) +
  "&lon=" +
  String(lon,6) +
  "&appid=" +
  apiKey +
  "&units=metric";

  Serial.println("Weather API:");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200)
  {
    String payload = http.getString();

    StaticJsonDocument<1024> doc;
    deserializeJson(doc, payload);

    float temp = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    Blynk.virtualWrite(V3, temp);
  }

  http.end();
}

void sendUptime()
{
  Blynk.virtualWrite(V0, millis() / 1000);
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, pass);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  getIPLocation();

  timer.setInterval(1000L, sendUptime);
  timer.setInterval(60000L, getWeather);
}

void loop()
{
  Blynk.run();
  timer.run();
}