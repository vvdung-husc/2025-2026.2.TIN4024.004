// Blynk
#define BLYNK_TEMPLATE_ID "TMPL60VV6-MLb"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "-paCzdHHiA6m7YcHti5AlWZUKjCRQjGN"
// Le Ngoc Minh

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// WiFi
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// OpenWeather API
//#define OPENWEATHERMAP_KEY "170055364c21bd4c1a8b3c0e8ef7daa2"

// Lưu thông tin IP
struct IP4_Info {
  String ip;
  String lat;
  String lon;
};

IP4_Info info;

String weatherURL;

unsigned long lastWeather = 0;
unsigned long lastUptime = 0;

void parseGeo(String payload)
{
  String data[7];
  int index = 0;

  while(payload.length() && index < 7)
  {
    int pos = payload.indexOf('|');

    if(pos == -1)
    {
      data[index++] = payload;
      break;
    }

    data[index++] = payload.substring(0,pos);
    payload = payload.substring(pos+1);
  }

  info.ip = data[0];
  info.lon = data[5];
  info.lat = data[6];

  Serial.println("====== GEO INFO ======");
  Serial.println("IP: " + info.ip);
  Serial.println("Latitude: " + info.lat);
  Serial.println("Longitude: " + info.lon);

  String maps = "https://www.google.com/maps/place/" + info.lat + "," + info.lon;

  Serial.println("Google Maps:");
  Serial.println(maps);

  Blynk.virtualWrite(V1, info.ip);
  Blynk.virtualWrite(V2, maps);

  weatherURL =
  "https://api.openweathermap.org/data/2.5/weather?lat=" +
  info.lat +
  "&lon=" + info.lon +
  "&appid=" + OPENWEATHERMAP_KEY +
  "&units=metric";

  Serial.println("Weather API:");
  Serial.println(weatherURL);
}

void getIPLocation()
{
  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int code = http.GET();

  if(code > 0)
  {
    String payload = http.getString();

    Serial.println(payload);

    parseGeo(payload);
  }
  else
  {
    Serial.println("API Error");
  }

  http.end();
}

void updateWeather()
{
  if(millis() - lastWeather < 10000) return;

  lastWeather = millis();

  HTTPClient http;

  http.begin(weatherURL);

  int code = http.GET();

  if(code > 0)
  {
    String payload = http.getString();

    Serial.println(payload);

    JsonDocument doc;

    deserializeJson(doc, payload);

    float temp = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    Blynk.virtualWrite(V3,temp);
  }

  http.end();
}

void updateUptime()
{
  if(millis() - lastUptime < 1000) return;

  lastUptime = millis();

  Blynk.virtualWrite(V0,millis()/1000);
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);

  Serial.print("Connecting WiFi");

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getIPLocation();
}

void loop()
{
  Blynk.run();

  updateWeather();

  updateUptime();
}