#include <Arduino.h>
#include <TM1637Display.h>

/* Blynk */
#define BLYNK_TEMPLATE_ID "TMPL6EGlnb4kY"
#define BLYNK_TEMPLATE_NAME "Api"
#define BLYNK_AUTH_TOKEN "SXEwYrU-VDzso93bMNOwCS0JikspwfgC"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

/* WIFI */
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* PIN */
#define btnBLED 23
#define pinBLED 21

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

/* TIMER */
unsigned long currentMiliseconds = 0;
bool blueButtonON = true;

/* STRUCT LƯU IP + VỊ TRÍ */
struct IP4_Info{
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;

/* WEATHER API */
#define OPENWEATHERMAP_KEY "0f746d56be5e86767777b5896e7b8f9e"

String urlWeather;

/* TIMER FUNCTION */
bool IsReady(unsigned long &timer, uint32_t ms){
  if (currentMiliseconds - timer < ms) return false;
  timer = currentMiliseconds;
  return true;
}

/* TÁCH DỮ LIỆU API IP */
void parseGeoInfo(String payload){
  String values[7];
  int index = 0;

  while(payload.length() > 0 && index < 7){
    int delimiter = payload.indexOf('|');

    if(delimiter == -1){
      values[index++] = payload;
      break;
    }

    values[index++] = payload.substring(0,delimiter);
    payload = payload.substring(delimiter + 1);
  }

  ip4Info.ip4 = values[0];
  ip4Info.longitude = values[5];
  ip4Info.latitude = values[6];

  Serial.print("IP: ");
  Serial.println(ip4Info.ip4);
}

/* LẤY IP + VỊ TRÍ */
void getAPI(){

  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");

  int code = http.GET();

  if(code == 200){

    String response = http.getString();

    parseGeoInfo(response);

    urlWeather =
    "http://api.openweathermap.org/data/2.5/weather?lat=" +
    ip4Info.latitude +
    "&lon=" +
    ip4Info.longitude +
    "&appid=" +
    OPENWEATHERMAP_KEY +
    "&units=metric";

    Serial.println(urlWeather);

    Blynk.virtualWrite(V1, ip4Info.ip4);

    String link =
    "https://www.google.com/maps/place/" +
    ip4Info.latitude + "," +
    ip4Info.longitude;

    Blynk.virtualWrite(V2, link);
  }

  http.end();
}

/* LẤY NHIỆT ĐỘ */
void updateTemp(){

  static unsigned long lastTime = 0;

  if(!IsReady(lastTime,10000)) return;
  if(urlWeather == "") return;
  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(urlWeather);

  int code = http.GET();

  if(code == 200){

    String response = http.getString();

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc,response);

    if(error){
      Serial.println("JSON ERROR");
      http.end();
      return;
    }

    float temp = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    Blynk.virtualWrite(V3,temp);
  }

  http.end();
}

/* THỜI GIAN HOẠT ĐỘNG */
void uptimeBlynk(){

  static unsigned long lastTime = 0;

  if(!IsReady(lastTime,1000)) return;

  unsigned long value = millis() / 1000;

  Blynk.virtualWrite(V0,value);

  if(blueButtonON){
    display.showNumberDec(value,false);
  }
}

/* NÚT BẤM */
void updateBlueButton(){

  static unsigned long lastTime = 0;
  static int lastValue = HIGH;

  if(!IsReady(lastTime,50)) return;

  int v = digitalRead(btnBLED);

  if(v == lastValue) return;

  lastValue = v;

  if(v == LOW) return;

  blueButtonON = !blueButtonON;

  digitalWrite(pinBLED,blueButtonON);

  Blynk.virtualWrite(V4,blueButtonON);

  if(!blueButtonON){
    display.clear();
  }
}

/* BLYNK BUTTON */
BLYNK_WRITE(V4){

  blueButtonON = param.asInt();
  digitalWrite(pinBLED,blueButtonON);

  if(!blueButtonON){
    display.clear();
  }
}

void setup(){

  Serial.begin(115200);

  pinMode(pinBLED,OUTPUT);
  pinMode(btnBLED,INPUT_PULLUP);

  display.setBrightness(7);

  Serial.println("Connecting WiFi...");

  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("Connecting Blynk...");

  Blynk.config(BLYNK_AUTH_TOKEN, "45.55.96.146", 80);

  while(!Blynk.connect()){
    Serial.println("Retry Blynk...");
    delay(1000);
  }

  Serial.println("Blynk Connected!");

  getAPI();
}

void loop(){

  Blynk.run();

  currentMiliseconds = millis();

  uptimeBlynk();

  updateBlueButton();

  updateTemp();
}