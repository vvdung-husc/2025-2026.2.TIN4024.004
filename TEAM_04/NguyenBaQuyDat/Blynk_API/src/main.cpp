#include <Arduino.h>

// Thông tin thiết bị Blynk của bạn
#define BLYNK_TEMPLATE_ID "TMPL6biIlrPrD"
#define BLYNK_TEMPLATE_NAME "ESP32API"
#define BLYNK_AUTH_TOKEN "Qe0SuB_QmocgJ_HxhTGMNlIyNL1U808A"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// Key OpenWeatherMap của bạn
#define OPENWEATHERMAP_KEY "0e460abbe0dd6cb0d6af4482c76ef17d"

struct IP4_Info
{
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;
String urlWeather;
unsigned long currentMiliseconds = 0;

// Hàm kiểm tra thời gian không dùng delay
bool IsReady(unsigned long &ulTimer, uint32_t milisecond)
{
  if (millis() - ulTimer < milisecond)
    return false;
  ulTimer = millis();
  return true;
}

// Hàm phân tích chuỗi: 123.25.115.160|VN|Viet Nam|Thua Thien-Hue|Hue|107.599998|16.466669
void parseGeoInfo(String payload, IP4_Info &ipInfo)
{
  String values[7];
  int index = 0;
  int lastDelimiter = 0;

  for (int i = 0; i < payload.length(); i++)
  {
    if (payload[i] == '|')
    {
      values[index++] = payload.substring(lastDelimiter, i);
      lastDelimiter = i + 1;
    }
    if (index == 6)
      break;
  }
  values[6] = payload.substring(lastDelimiter); // Lấy phần cuối cùng (Latitude)

  ipInfo.ip4 = values[0];
  ipInfo.longtitude = values[5];
  ipInfo.latitude = values[6];

  Serial.println("--- GEO INFO ---");
  Serial.printf("IP: %s\n", ipInfo.ip4.c_str());
  Serial.printf("Lat: %s, Lon: %s\n", ipInfo.latitude.c_str(), ipInfo.longtitude.c_str());
}

// Lấy thông tin IP và tọa độ
void getGeoAPI()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("http://ip4.iothings.vn/?geo=1");
    int httpCode = http.GET();

    if (httpCode > 0)
    {
      String response = http.getString();
      parseGeoInfo(response, ip4Info);

      // Tạo URL cho OpenWeatherMap
      urlWeather = "https://api.openweathermap.org/data/2.5/weather?lat=" + ip4Info.latitude +
                   "&lon=" + ip4Info.longtitude + "&appid=" + OPENWEATHERMAP_KEY + "&units=metric";
    }
    else
    {
      Serial.println("Error on Geo API");
    }
    http.end();
  }
}

// Lấy nhiệt độ và gửi lên V3
void updateWeather()
{
  static unsigned long lastTempTime = 0;
  if (!IsReady(lastTempTime, 10000))
    return; // Cập nhật mỗi 10 giây

  if (WiFi.status() == WL_CONNECTED && urlWeather.length() > 0)
  {
    HTTPClient http;
    http.begin(urlWeather);
    int httpCode = http.GET();

    if (httpCode == 200)
    {
      String response = http.getString();
      JsonDocument doc;
      deserializeJson(doc, response);
      float temp = doc["main"]["temp"];

      Serial.printf("Temp: %.2f C\n", temp);
      Blynk.virtualWrite(V3, temp); // Gửi nhiệt độ lên Gauge V3
    }
    http.end();
  }
}

// Gửi IP (V1) và Link Google Maps (V2) một lần duy nhất
void sendStaticInfo()
{
  static bool sent = false;
  if (sent || ip4Info.ip4.length() == 0)
    return;

  String googleMapsLink = "https://www.google.com/maps?q=" + ip4Info.latitude + "," + ip4Info.longtitude;

  Blynk.virtualWrite(V1, ip4Info.ip4);    // Hiển thị IPv4 lên V1
  Blynk.virtualWrite(V2, googleMapsLink); // Hiển thị Link lên V2

  Serial.println("Static Info Sent to Blynk");
  sent = true;
}

// Cập nhật thời gian hoạt động lên V0
void updateUptime()
{
  static unsigned long lastUptime = 0;
  if (!IsReady(lastUptime, 1000))
    return;
  Blynk.virtualWrite(V0, millis() / 1000); // Gửi số giây lên V0
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getGeoAPI(); // Gọi lấy tọa độ ngay khi khởi động
}

void loop()
{
  Blynk.run();

  updateUptime();
  sendStaticInfo();
  updateWeather();
}