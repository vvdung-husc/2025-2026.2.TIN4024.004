#include <Arduino.h>

// thông tin template Blynk
#define BLYNK_TEMPLATE_ID "TMPL6Ub2n9rxe"
#define BLYNK_TEMPLATE_NAME "BLYNK API"
#define BLYNK_AUTH_TOKEN "bqLfUbhNqPQZxTptjtKgpn7ExqoabEWU"

// thư viện cần thiết
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// cấu hình WiFi (Wokwi)
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

// struct lưu thông tin IP và vị trí
struct IP4_Info {
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;

// biến dùng cho timer
ulong currentMiliseconds = 0;

// hàm kiểm tra timer để chạy theo chu kỳ
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// hàm format chuỗi giống printf
String StringFormat(const char* fmt, ...) {

  va_list args;
  va_start(args, fmt);

  char buffer[200];
  vsnprintf(buffer, sizeof(buffer), fmt, args);

  va_end(args);

  return String(buffer);
}

// ---------------------------------------------------
// phân tích dữ liệu API IP
// ---------------------------------------------------

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

  // gán dữ liệu
  ipInfo.ip4 = values[0];
  ipInfo.latitude = values[6];
  ipInfo.longitude = values[5];
}

// API key thời tiết từ
// :contentReference[oaicite:1]{index=1}
#define OPENWEATHERMAP_KEY "40438fa01109ad143d661574d1825764"

String urlWeather;

// ---------------------------------------------------
// hàm lấy IP và tọa độ
// ---------------------------------------------------

void getAPI() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.println("== DATA IP API ==");
    Serial.println(response);

    // phân tích dữ liệu
    parseGeoInfo(response, ip4Info);

    // tạo link Google Map
    String urlGoogleMaps =
      StringFormat("https://www.google.com/maps/place/%s,%s",
                   ip4Info.latitude.c_str(),
                   ip4Info.longitude.c_str());

    Serial.printf("IPv4 => %s\n", ip4Info.ip4.c_str());
    Serial.println(urlGoogleMaps);

    // tạo URL gọi API thời tiết
    urlWeather =
      StringFormat(
        "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
        ip4Info.latitude.c_str(),
        ip4Info.longitude.c_str(),
        OPENWEATHERMAP_KEY);

    Serial.println(urlWeather);
  }

  http.end();
}

// ---------------------------------------------------
// hàm cập nhật nhiệt độ
// ---------------------------------------------------

void updateTemp() {

  static ulong lastTime = 0;
  static float temp_ = 0;

  // cập nhật mỗi 10 giây
  if (!IsReady(lastTime, 10000)) return;

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  http.begin(urlWeather);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {

    String response = http.getString();

    StaticJsonDocument<1024> doc;

    DeserializationError error =
      deserializeJson(doc, response);

    if (!error) {

      float temp = doc["main"]["temp"];

      if (temp_ != temp) {

        temp_ = temp;

        Serial.print("Temperature: ");
        Serial.println(temp_);

        // gửi lên Blynk
        Blynk.virtualWrite(V1, temp_);
      }
    }
  }

  http.end();
}

// ---------------------------------------------------
// gửi IP và Google Maps lên Blynk (chỉ 1 lần)
// ---------------------------------------------------

void onceCalled() {

  static bool done_ = false;

  if (done_) return;

  done_ = true;

  String link =
    StringFormat("https://www.google.com/maps/place/%s,%s",
                 ip4Info.latitude.c_str(),
                 ip4Info.longitude.c_str());

  Blynk.virtualWrite(V4, ip4Info.ip4);
  Blynk.virtualWrite(V5, link);
}

// ---------------------------------------------------
// gửi thời gian hoạt động (uptime)
// ---------------------------------------------------

void uptimeBlynk() {

  static ulong lastTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  ulong value = lastTime / 1000;

  Blynk.virtualWrite(V3, value);
}

// ---------------------------------------------------
// setup
// ---------------------------------------------------

void setup() {

  Serial.begin(115200);

  // kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  Serial.print("Connecting to WiFi ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println(" Connected!");

  // kết nối Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  // gọi API lấy vị trí
  getAPI();
}

// ---------------------------------------------------
// loop chính
// ---------------------------------------------------

void loop() {

  Blynk.run();

  currentMiliseconds = millis();

  onceCalled();   // gửi IP + map
  updateTemp();   // cập nhật nhiệt độ
  uptimeBlynk();  // gửi uptime
}