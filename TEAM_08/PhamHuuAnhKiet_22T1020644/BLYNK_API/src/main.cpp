#define BLYNK_TEMPLATE_ID "TMPL6maKop6k2"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "MxLagMP0qVpW0A565pPmU8jsoxywHO2f"

#include <Arduino.h> // Bắt buộc phải có khi dùng PlatformIO
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";

String apiKey = "5a5c4a9fec1b2237889827fd747858d6";

float lat = 0.0;
float lon = 0.0;

BlynkTimer timer;

void getIPLocation()
{
  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();
    Serial.println("--- API IP/Location Response ---");
    Serial.println(payload);

    // Tách chuỗi theo logic cũ của bạn
    int p1 = payload.indexOf('|');
    int p6 = payload.lastIndexOf('|');

    String ip = payload.substring(0, p1);
    String latStr = payload.substring(p6 + 1);

    int p5 = payload.lastIndexOf('|', p6 - 1);
    String lonStr = payload.substring(p5 + 1, p6);

    lat = latStr.toFloat();
    lon = lonStr.toFloat();

    // Sửa lại format chuẩn xác 100% theo yêu cầu của ảnh đề bài
    String googleMap = "http://www.google.com/maps/place/" + String(lat, 6) + "," + String(lon, 6);

    Serial.println("IP: " + ip);
    Serial.println("Google Maps: " + googleMap);

    Blynk.virtualWrite(V1, ip);
    Blynk.virtualWrite(V2, googleMap);
  }
  else
  {
    Serial.println("Lỗi gọi API IP/Location");
  }

  http.end();
}

void getWeather()
{
  // Chỉ gọi API thời tiết nếu đã lấy được tọa độ thành công
  if (lat == 0.0 && lon == 0.0) {
    Serial.println("Chưa có tọa độ, bỏ qua cập nhật thời tiết.");
    return;
  }

  HTTPClient http;
  String url = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(lat, 6) + "&lon=" + String(lon, 6) + "&appid=" + apiKey + "&units=metric";

  Serial.println("--- API Weather ---");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();

    // Dùng JsonDocument thay cho StaticJsonDocument (Cú pháp chuẩn của ArduinoJson v7)
    JsonDocument doc; 
    DeserializationError error = deserializeJson(doc, payload);

    if (!error)
    {
      float temp = doc["main"]["temp"];
      Serial.print("Temperature: ");
      Serial.print(temp);
      Serial.println(" °C");

      Blynk.virtualWrite(V3, temp);
    }
    else
    {
      Serial.print("Lỗi phân tích JSON: ");
      Serial.println(error.c_str());
    }
  }
  else
  {
    Serial.println("Lỗi gọi API Thời tiết");
  }

  http.end();
}

void sendUptime()
{
  Blynk.virtualWrite(V0, millis() / 1000);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Khởi động...");

  // Hàm Blynk.begin đã bao trọn gói việc kết nối WiFi, không cần dùng vòng lặp while(WiFi.status) nữa
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Đã kết nối Blynk và WiFi!");

  // Gọi ngay để hiển thị thông tin lần đầu tiên lúc vừa khởi động
  getIPLocation();
  getWeather();

  // Thiết lập các tác vụ định kỳ
  timer.setInterval(1000L, sendUptime);    // Cập nhật Uptime mỗi 1 giây
  timer.setInterval(60000L, getWeather);   // Cập nhật thời tiết mỗi 60 giây
}

void loop()
{
  Blynk.run();
  timer.run();
}