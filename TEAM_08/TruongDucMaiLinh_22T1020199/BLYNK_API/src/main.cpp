#include <Arduino.h>
#include "config.h"


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


struct IP4_Info{
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;

unsigned long currentMiliseconds = 0;
String urlWeather;

//#define OPENWEATHERMAP_KEY "29975be309f32b6598d4fcb2883ae0bb"

bool IsReady(unsigned long &timer, uint32_t ms)
{
  if (currentMiliseconds - timer < ms) return false;
  timer = currentMiliseconds;
  return true;
}

String StringFormat(const char* fmt, ...){
  char buffer[200];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  return String(buffer);
}

void parseGeoInfo(String payload, IP4_Info& ipInfo) {
  String values[7];
  int index = 0;

  while (payload.length() > 0 && index < 7) {
    int pos = payload.indexOf('|');

    if (pos == -1) {
      values[index++] = payload;
      break;
    }

    values[index++] = payload.substring(0, pos);
    payload = payload.substring(pos + 1);
  }

  ipInfo.ip4 = values[0];
  ipInfo.longitude = values[5];
  ipInfo.latitude = values[6];
}

void getAPI(){

  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpCode = http.GET();

  if(httpCode > 0){

    String response = http.getString();

    Serial.println("DATA IP API:");
    Serial.println(response);

    parseGeoInfo(response, ip4Info);

    String mapLink =
    StringFormat("https://www.google.com/maps/place/%s,%s",
    ip4Info.latitude.c_str(),
    ip4Info.longitude.c_str());

    Serial.println(mapLink);

    urlWeather =
    StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
    ip4Info.latitude.c_str(),
    ip4Info.longitude.c_str(),
    OPENWEATHERMAP_KEY);

    Blynk.virtualWrite(V1, ip4Info.ip4);
    Blynk.virtualWrite(V2, mapLink);
  }

  http.end();
}

void updateTemp(){

  static unsigned long lastTime = 0;

  if(!IsReady(lastTime,10000)) return;

  if(WiFi.status()!=WL_CONNECTED) return;

  HTTPClient http;

  http.begin(urlWeather);

  int httpCode = http.GET();

  if(httpCode>0){

    String response = http.getString();

    DynamicJsonDocument doc(2048);

    deserializeJson(doc,response);

    float temp = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    Blynk.virtualWrite(V3,temp);
  }

  http.end();
}

void uptimeBlynk(){

  static unsigned long lastTime = 0;

  if(!IsReady(lastTime,1000)) return;

  unsigned long value = lastTime/1000;

  Blynk.virtualWrite(V0,value);
}

void setup(){

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID,WIFI_PASSWORD,WIFI_CHANNEL);

  while(WiFi.status()!=WL_CONNECTED){
    delay(100);
    Serial.print(".");
  }

  Serial.println("WiFi Connected");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}

void loop(){

  Blynk.run();

  currentMiliseconds = millis();

  updateTemp();

  uptimeBlynk();
}