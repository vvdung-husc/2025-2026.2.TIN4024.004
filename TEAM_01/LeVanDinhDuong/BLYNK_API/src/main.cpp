#include <Arduino.h>

// Thông số BLYNK 
#define BLYNK_TEMPLATE_ID "TMPL60iL3cLu-"
#define BLYNK_TEMPLATE_NAME "ESP32 API Project"
#define BLYNK_AUTH_TOKEN "KEXiKgY-KxWwPHMtKD6Nm7mJhfvOfCzT"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>   
#include <ArduinoJson.h>  

// Cấu hình WiFi
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;
unsigned long currentMiliseconds = 0; 
bool isApiReady = false; 
String urlWeather;

// API Key OpenWeatherMap bạn cung cấp
#define OPENWEATHERMAP_KEY "8c05d3c22d99e4c31bcca02503d158e5" 

// Hàm kiểm tra thời gian không dùng delay()
bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (millis() - ulTimer < milisecond) return false;
  ulTimer = millis();
  return true;
}

// Phân tích chuỗi trả về từ iothings (IP|Country|City|...|Lon|Lat)
void parseGeoInfo(String payload, IP4_Info& ipInfo) {
  String values[7];
  int index = 0;
  int lastDelimiter = 0;
  
  for (int i = 0; i < payload.length() && index < 7; i++) {
    if (payload[i] == '|') {
      values[index++] = payload.substring(lastDelimiter, i);
      lastDelimiter = i + 1;
    }
  }
  if (index < 7) values[index] = payload.substring(lastDelimiter);

  ipInfo.ip4 = values[0];
  ipInfo.longtitude = values[5]; 
  ipInfo.latitude = values[6];   
  
  Serial.println("--- Da lay thong tin vi tri ---");
  Serial.printf("IP: %s | Lat: %s | Lon: %s\n", ipInfo.ip4.c_str(), ipInfo.latitude.c_str(), ipInfo.longtitude.c_str());
}

// Lấy IP và Tọa độ từ iothings
void getAPI() {
  if(WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  
  int httpResponseCode = http.GET();
  if(httpResponseCode == 200) {
    String response = http.getString();
    parseGeoInfo(response, ip4Info);

    // Tao link Google Maps (V2)
    String googleMapsLink = "https://www.google.com/maps/place/" + ip4Info.latitude + "," + ip4Info.longtitude;
    
    // Day len Blynk ngay lap tuc
    Blynk.virtualWrite(V1, ip4Info.ip4);
    Blynk.virtualWrite(V2, googleMapsLink);
    
    // Tao URL Weather cho OpenWeatherMap
    urlWeather = "https://api.openweathermap.org/data/2.5/weather?lat=" + ip4Info.latitude + 
                 "&lon=" + ip4Info.longtitude + 
                 "&appid=" + String(OPENWEATHERMAP_KEY) + "&units=metric";
    
    isApiReady = true; 
    Serial.println("Google Maps: " + googleMapsLink);
  } else {
    Serial.printf("Loi lay IP/Geo: %d\n", httpResponseCode);
  }
  http.end();
}

// Lay nhiet do tu OpenWeatherMap (V3)
void updateTemp() {
  static unsigned long lastTime = 0;
  static uint32_t interval = 5000; 

  if (!IsReady(lastTime, interval) || urlWeather == "") return; 
  interval = 60000; 

  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;   
  http.begin(urlWeather);
  
  int httpResponseCode = http.GET();
  if(httpResponseCode == 200) {
    String response = http.getString();
    
    // --- SỬA TẠI ĐÂY ---
    JsonDocument doc; // Thay StaticJsonDocument<1024> bằng JsonDocument
    // Thư viện V7 sẽ tự động quản lý bộ nhớ, không cần khai báo kích thước <1024>
    
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
      Serial.print("Lỗi phân tích JSON: ");
      Serial.println(error.f_str());
    } else {
      float temp = doc["main"]["temp"]; 
      Serial.printf("Nhiệt độ hiện tại: %.1f C\n", temp);
      Blynk.virtualWrite(V3, temp); 
    }
  } else {
    Serial.printf("Lỗi lấy thời tiết: %d\n", httpResponseCode);
  }
  http.end();
}

// Cap nhat thoi gian hoat dong (V0)
void uptimeBlynk() {
  static unsigned long lastUptime = 0;
  if (!IsReady(lastUptime, 1000)) return; 
  Blynk.virtualWrite(V0, millis() / 1000);  
}

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN); 
  Blynk.connect();                

  getAPI(); // Lay thong tin vi tri ngay khi bat dau
}

void loop() {
  Blynk.run();  
  
  if (!isApiReady) {
    static unsigned long retryTime = 0;
    if (IsReady(retryTime, 5000)) getAPI();
  } else {
    updateTemp();
  }
  
  uptimeBlynk();
}