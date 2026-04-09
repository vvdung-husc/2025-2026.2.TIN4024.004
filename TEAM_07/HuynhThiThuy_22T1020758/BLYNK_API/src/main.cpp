#include <Arduino.h>

//===== BLYNK =====
#define BLYNK_TEMPLATE_ID "xx"
#define BLYNK_TEMPLATE_NAME "xx"
#define BLYNK_AUTH_TOKEN "xx"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//===== WIFI =====
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

//===== OpenWeather =====
#define OPENWEATHERMAP_KEY "xx"

//===== STRUCT LƯU IP + LAT + LON =====
struct IP4_Info{
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;

String urlWeather;
ulong currentMiliseconds = 0;

//===== TIMER KHÔNG DÙNG DELAY =====
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

//===== FORMAT STRING =====
String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

//===== PARSE GEO INFO =====
void parseGeoInfo(String payload, IP4_Info& ipInfo) {
  String values[7];
  int index = 0;

  while (payload.length() > 0 && index < 7) {
    int delimiterIndex = payload.indexOf('|');

    if (delimiterIndex == -1) {
      values[index++] = payload;
      break;
    }

    values[index++] = payload.substring(0, delimiterIndex);
    payload = payload.substring(delimiterIndex + 1);
  }

  ipInfo.ip4 = values[0];
  ipInfo.latitude = values[6];
  ipInfo.longtitude = values[5];

  Serial.println("IP: " + ipInfo.ip4);
  Serial.println("Lat: " + ipInfo.latitude);
  Serial.println("Lon: " + ipInfo.longtitude);
}

//===== GET IP + LAT + LON =====
void getAPI(){
  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpResponseCode = http.GET();

  if(httpResponseCode>0){
    String response = http.getString();
    parseGeoInfo(response, ip4Info);

    String urlGooleMaps = StringFormat(
      "https://www.google.com/maps/place/%s,%s",
      ip4Info.latitude.c_str(),
      ip4Info.longtitude.c_str()
    );

    urlWeather = StringFormat(
      "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
      ip4Info.latitude.c_str(),
      ip4Info.longtitude.c_str(),
      OPENWEATHERMAP_KEY
    );

    // Gửi lên Blynk
    Blynk.virtualWrite(V1, ip4Info.ip4);
    Blynk.virtualWrite(V2, urlGooleMaps);
  }

  http.end();
}

//===== UPDATE NHIỆT ĐỘ =====
void updateTemp(){
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 60000)) return; // cập nhật mỗi 60s

  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;   
  http.begin(urlWeather);

  int httpResponseCode = http.GET();

  if(httpResponseCode>0){
    String response = http.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (!error) {
      float temp = doc["main"]["temp"];

      if (temp_ != temp){
        temp_ = temp;
        Serial.println("Nhiet do: " + String(temp));
        Blynk.virtualWrite(V3, temp_);
      }
    }
  }

  http.end();
}

//===== UPTIME =====
void uptimeBlynk(){
  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000)) return;

  ulong seconds = millis() / 1000;
  Blynk.virtualWrite(V0, seconds);
}

//===== SETUP =====
void setup(void) {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI(); // gọi 1 lần
}

//===== LOOP =====
void loop(void) {
  Blynk.run();

  currentMiliseconds = millis();

  uptimeBlynk();
  updateTemp();
}