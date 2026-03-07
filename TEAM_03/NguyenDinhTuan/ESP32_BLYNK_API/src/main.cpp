#include <Arduino.h>
#include <TM1637Display.h>

/* Blynk */
#define BLYNK_TEMPLATE_ID "TMPL6lGFdsfCS"
#define BLYNK_TEMPLATE_NAME "IOT TEMPLATE"
#define BLYNK_AUTH_TOKEN "YPy5vGILJQdifdYBigo0-Nn_olk54-SP"

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

/*STRUCT LƯU THÔNG TIN IP + VỊ TRÍ*/

struct IP4_Info{
  String ip4;        // địa chỉ IPv4 của thiết bị
  String latitude;   // vĩ độ
  String longitude;  // kinh độ
};

IP4_Info ip4Info;

/* WEATHER API */
#define OPENWEATHERMAP_KEY "YOUR_API_KEY_HERE"

String urlWeather;


bool IsReady(unsigned long &timer, uint32_t ms){
  if (currentMiliseconds - timer < ms) return false;
  timer = currentMiliseconds;
  return true;
}

/*TÁCH DỮ LIỆU TRẢ VỀ TỪ API IP */

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

  /* LẤY THÔNG TIN CẦN THIẾT */

  ip4Info.ip4 = values[0];        // IPv4
  ip4Info.longitude = values[5];  // kinh độ
  ip4Info.latitude = values[6];   // vĩ độ

  Serial.print("IP: ");
  Serial.println(ip4Info.ip4);
}

/* LẤY IPv4 + VỊ TRÍ ĐỊA LÝ*/

void getAPI(){

  HTTPClient http;

  // gọi API lấy IPv4 và vị trí
  http.begin("http://ip4.iothings.vn/?geo=1");
  int code = http.GET();

  if(code == 200){

    // nhận dữ liệu trả về
    String response = http.getString();

    // tách dữ liệu 
    parseGeoInfo(response);

    /*TẠO URL API LẤY THỜI TIẾT*/

    urlWeather =
    "http://api.openweathermap.org/data/2.5/weather?lat=" +
    ip4Info.latitude +
    "&lon=" +
    ip4Info.longitude +
    "&appid=" +
    OPENWEATHERMAP_KEY +
    "&units=metric";
    Serial.println(urlWeather);
    /*GỬI IPv4 LÊN BLYNK*/

    Blynk.virtualWrite(V1, ip4Info.ip4);

    /*TẠO LINK GOOGLE MAPS */

    String link =
    "https://www.google.com/maps/place/" +
    ip4Info.latitude + "," +
    ip4Info.longitude;

    // gửi link lên Blynk
    Blynk.virtualWrite(V2, link);
  }

  http.end();
}

/*LẤY NHIỆT ĐỘ TỪ OPENWEATHERMAP*/

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

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc,response);

    if(error){
      Serial.println("JSON ERROR");
      http.end();
      return;
    }

    // lấy nhiệt độ trong JSON
    float temp = doc["main"]["temp"];

    Serial.print("Temperature: ");
    Serial.println(temp);

    // gửi nhiệt độ lên Blynk
    Blynk.virtualWrite(V3,temp);
  }

  http.end();
}



void uptimeBlynk(){

  static unsigned long lastTime = 0;

  if(!IsReady(lastTime,1000)) return;

  unsigned long value = lastTime / 1000;

  Blynk.virtualWrite(V0,value);

  if(blueButtonON){
    display.showNumberDec(value);
  }
}



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

  Serial.println("Connecting Blynk...");

  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);

  Serial.println("Connected");

  // lấy IPv4 + vị trí khi khởi động
  getAPI();
}


void loop(){

  Blynk.run();

  currentMiliseconds = millis();

  uptimeBlynk();

  updateBlueButton();

  updateTemp();
}