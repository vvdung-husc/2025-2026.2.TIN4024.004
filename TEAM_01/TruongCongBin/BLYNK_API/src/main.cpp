#include <Arduino.h>

// Thông số BLYNK
#define BLYNK_TEMPLATE_ID "TMPL6R6s5BH-D"
#define BLYNK_TEMPLATE_NAME "BLYNK API"
#define BLYNK_AUTH_TOKEN "eTFaM09VlGtl_RPeku-NW0Fk0sy5_R-f"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>   
#include <ArduinoJson.h>  

// NẾU DÙNG WOKWI: Giữ nguyên. NẾU DÙNG MẠCH THẬT: Sửa tên và pass WiFi nhà bạn vào đây!
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
bool isApiReady = false; 

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Hàm format chuỗi
String StringFormat(const char* fmt, ...) {
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

// Phân tích chuỗi trả về từ iothings
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
  ipInfo.longtitude = values[5]; 
  ipInfo.latitude = values[6];   
  
  Serial.printf("IP Address: %s\r\n", values[0].c_str());
  Serial.printf("Longitude: %s\r\n", values[5].c_str());
  Serial.printf("Latitude: %s\r\n", values[6].c_str());
}

// Key OpenWeatherMap của bạn
#define OPENWEATHERMAP_KEY "1fcd4fde730e52481fb488a291fa1a23" 
String urlWeather;

// Lấy API IP và Tọa độ
void getAPI() {
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Lỗi: Mất kết nối WiFi"); return;
  }
  
  HTTPClient http;   
  http.begin("http://ip4.iothings.vn/?geo=1");
  
  int httpResponseCode = http.GET();
  if(httpResponseCode > 0) {
    String response = http.getString();
    parseGeoInfo(response, ip4Info);

    // ĐÃ SỬA LỖI: Định dạng link bản đồ chuẩn xác với 2 biến %s
    String urlGooleMaps = StringFormat("https://www.google.com/maps?q=%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    Serial.printf("IPv4 => %s \r\n", ip4Info.ip4.c_str());
    Serial.println(urlGooleMaps);

    // Tạo link thời tiết
    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str(), OPENWEATHERMAP_KEY);
    Serial.printf("URL Weather => %s \r\n", urlWeather.c_str());      
    
    isApiReady = true; 
  } else {
    Serial.print("Lỗi HTTP GET (IP): ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Cập nhật nhiệt độ
void updateTemp() {
  static ulong lastTime = 0;
  static float temp_ = -999.0; 

  // Lấy nhiệt độ mỗi 60 giây
  if (!IsReady(lastTime, 60000)) return; 
  if(WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;   
  http.begin(urlWeather);
  
  int httpResponseCode = http.GET();
  if(httpResponseCode > 0) {
    String response = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
      Serial.println("Lỗi phân tích JSON Thời tiết");
    } else {
      float temp = doc["main"]["temp"]; 
      if (temp_ != temp) {
        temp_ = temp;
        Serial.print("Nhiet do hien tai: "); Serial.println(temp); 
        Blynk.virtualWrite(V3, temp_);
      }
    }
  } else {
    Serial.print("Lỗi HTTP GET (Weather): ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Gửi IP và Link Maps lên Blynk 1 lần
void onceCalled() {
  static bool done_ = false;
  if (done_) return;
  done_ = true;
  
  // ĐÃ SỬA LỖI: Định dạng link bản đồ chuẩn xác với 2 biến %s
  String link = StringFormat("https://www.google.com/maps?q=%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());

  Blynk.virtualWrite(V1, ip4Info.ip4.c_str());  
  Blynk.virtualWrite(V2, link.c_str());  
}

// Cập nhật thời gian hoạt động
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; 
  
  ulong value = currentMiliseconds / 1000; 
  Blynk.virtualWrite(V0, value);  
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  
  Serial.print("Dang ket noi WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
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
  
  // Xử lý chống treo nếu rớt mạng lúc khởi động
  if (!isApiReady) {
    static ulong retryTime = 0;
    if (IsReady(retryTime, 5000)) {
      Serial.println("Dang thu lay lai API Toa do...");
      getAPI();
    }
  } else {
    onceCalled(); 
    updateTemp();
  }
  
  uptimeBlynk();
}