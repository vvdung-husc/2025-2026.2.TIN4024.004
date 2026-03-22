#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define BLYNK_TEMPLATE_ID "TMPL6A7APQs3E"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "xx"

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define OPENWEATHERMAP_KEY ""

unsigned long lastTemp = 0;
unsigned long lastUptime = 0;

String ipAddress = "";
String lat = "";
String lon = "";
String weatherURL = "";


// ===== LẤY GEO IP =====
void getIP(){
  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");

  int code = http.GET();

  if(code > 0){
    String payload = http.getString();
    Serial.println(payload);

    int p1 = payload.indexOf('|');
    int p2 = payload.indexOf('|', p1+1);
    int p3 = payload.indexOf('|', p2+1);
    int p4 = payload.indexOf('|', p3+1);
    int p5 = payload.indexOf('|', p4+1);
    int p6 = payload.indexOf('|', p5+1);

    ipAddress = payload.substring(0,p1);
    lon = payload.substring(p5+1,p6);
    lat = payload.substring(p6+1);

    Serial.println("IP: " + ipAddress);
    Serial.println("LAT: " + lat);
    Serial.println("LON: " + lon);

    String mapLink = "https://www.google.com/maps/place/" + lat + "," + lon;

    Blynk.virtualWrite(V1, ipAddress);
    Blynk.virtualWrite(V2, mapLink);

    weatherURL =
    "https://api.openweathermap.org/data/2.5/weather?lat="
    + lat +
    "&lon=" + lon +
    "&appid=" + OPENWEATHERMAP_KEY +
    "&units=metric";
  }

  http.end();
}


// ===== LẤY NHIỆT ĐỘ =====
void getTemp(){

  if(millis() - lastTemp < 10000) return;
  lastTemp = millis();

  if(weatherURL == "") return;
  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(weatherURL);

  int code = http.GET();

  if(code > 0){
    String payload = http.getString();

    JsonDocument doc;
    deserializeJson(doc, payload);

    float t = doc["main"]["temp"];

    Serial.print("Temp: ");
    Serial.println(t);

    Blynk.virtualWrite(V3, t);
  }

  http.end();
}


// ===== UPTIME =====
void sendUptime(){

  if(millis() - lastUptime < 1000) return;
  lastUptime = millis();

  Blynk.virtualWrite(V0, millis()/1000);
}


void setup(){
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println(" OK");

  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  Blynk.connect();
}


void loop(){

  Blynk.run();

  if(ipAddress == ""){
    getIP();
  }

  getTemp();
  sendUptime();
}