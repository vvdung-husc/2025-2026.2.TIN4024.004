#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6opXQI0CF"
#define BLYNK_TEMPLATE_NAME "Châu Văn Trường Huy"
#define BLYNK_AUTH_TOKEN "F4gnw7vRdGwCdaR5MgG5FgdfKn00t9_3"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>   //Thư viện gọi API
#include <ArduinoJson.h>  //Thư viện xử lý JSON

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

struct IP4_Info{
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;

ulong currentMiliseconds = 0; 

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

//Định dạng chuỗi %s,%s,...
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

//Phân tích chuỗi trả về từ http://ip4.iothings.vn/?geo=1 và điền vào ipInfo
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
  ipInfo.latitude = values[6].c_str();
  ipInfo.longtitude = values[5].c_str();
}

#define OPENWEATHERMAP_KEY "40438fa01109ad143d661574d1825764" 
String urlWeather;  

//API Get http://ip4.iothings.vn/?geo=1
void getAPI(){
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error in WiFi connection"); return;
  }
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.GET();
  if(httpResponseCode>0){
    String response = http.getString();
    Serial.println("== DATA IP API ==");
    Serial.println(response);
          
    parseGeoInfo(response, ip4Info);

    //Thêm định dạng link Google Maps 
    String urlGooleMaps = StringFormat("https://www.google.com/maps/place/%s,%s",ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.printf("IPv4 => %s \r\n",ip4Info.ip4.c_str());
    Serial.println(urlGooleMaps.c_str());

    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",ip4Info.latitude.c_str(),ip4Info.longtitude.c_str(),OPENWEATHERMAP_KEY);

    Serial.printf("URL Weather => %s \r\n",urlWeather.c_str());      
  }else{
    Serial.print("Error on sending GET IP: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

//Cập nhật nhiêt độ từ urlWeather bằng API GET
void updateTemp(){
  static ulong lastTime = 0;
  static float temp_ = 0.0;

  if (!IsReady(lastTime, 10000)) return; 
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("updateTemp() Error in WiFi connection"); 
    return;
  }

  HTTPClient http;   
  http.begin(urlWeather);
  http.addHeader("Content-Type", "text/plain");
  
  int httpResponseCode = http.GET();
  if(httpResponseCode>0){
    String response = http.getString();
          
    //Xử lý JSON trả về từ API
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.println("Failed to parse JSON");
    }
    else {
      float temp = doc["main"]["temp"];
      
      if (temp_ != temp){
        temp_ = temp;
        Serial.print("Nhiet do: "); Serial.println(temp); 
        Blynk.virtualWrite(V3, temp_);
      }
    }
  }else{
    Serial.print("Error on sending GET Weather: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

//Chỉ gọi 1 lần để cập nhật IPv4, Link GoogleMaps
void onceCalled(){
  static bool done_ = false;
  if (done_) return;
  done_ = true;
  
  //format link
  String link = StringFormat("https://www.google.com/maps/place/%s,%s",ip4Info.latitude.c_str(),ip4Info.longtitude.c_str());

  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());  
  Blynk.virtualWrite(V2, link.c_str());  
}

//Cập nhật uptime lên Blynk
void uptimeBlynk(){
  static ulong lastTime = 0;
  
  if (!IsReady(lastTime, 1000)) return; 
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);  
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN); 
  Blynk.connect();                

  getAPI();
}

void loop(void) {
  
  Blynk.run();  
  
  currentMiliseconds = millis();
  onceCalled(); 
  updateTemp();
  uptimeBlynk();
}