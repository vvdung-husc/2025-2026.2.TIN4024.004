#include <Arduino.h>

#define BLYNK_TEMPLATE_ID   "TMPL6lGFdsfCS"
#define BLYNK_TEMPLATE_NAME "IOT TEMPLATE"
#define BLYNK_AUTH_TOKEN    "YPy5vGILJQdifdYBigo0-Nn_olk54-SP"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""

struct IP4_Info {
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;
ulong currentMiliseconds = 0;

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
  return s;
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

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

  ipInfo.ip4       = values[0];
  ipInfo.longitude = values[5];   // index 5 = Longitude
  ipInfo.latitude  = values[6];   // index 6 = Latitude

  Serial.printf("IP Address  : %s\r\n", values[0].c_str());
  Serial.printf("Country Code: %s\r\n", values[1].c_str());
  Serial.printf("Country     : %s\r\n", values[2].c_str());
  Serial.printf("Region      : %s\r\n", values[3].c_str());
  Serial.printf("City        : %s\r\n", values[4].c_str());
  Serial.printf("Longitude   : %s\r\n", values[5].c_str());
  Serial.printf("Latitude    : %s\r\n", values[6].c_str());
}

// OpenWeatherMap API key 
#define OPENWEATHERMAP_KEY "YOUR_OPENWEATHER_API_KEY"  // <-- e5fbf5f20a3b8481bbe8a5cf79dea48c YOUR_OPENWEATHER_API_KEY
String urlWeather;

// GET http://ip4.iothings.vn/?geo=1 
void getAPI() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("getAPI() Error: WiFi not connected"); return;
  }

  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");
  http.addHeader("Content-Type", "text/plain");

  int code = http.GET();
  if (code > 0) {
    String response = http.getString();
    Serial.println(code);
    Serial.println(response);

    parseGeoInfo(response, ip4Info);

    String urlGoogleMaps = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longitude.c_str());
    Serial.printf("IPv4      => %s\r\n", ip4Info.ip4.c_str());
    Serial.printf("Maps link => %s\r\n", urlGoogleMaps.c_str());

    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric", ip4Info.latitude.c_str(), ip4Info.longitude.c_str(), OPENWEATHERMAP_KEY);
    Serial.printf("Weather URL => %s\r\n", urlWeather.c_str());
  } else {
    Serial.printf("getAPI() HTTP error: %d\r\n", code);
  }
  http.end();
}

// Cập nhật nhiệt độ từ OpenWeatherMap mỗi 10 giây 
void updateTemp() {
  static ulong lastTime = 0;
  static float temp_    = 0.0f;

  if (!IsReady(lastTime, 10000)) return;
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("updateTemp() Error: WiFi not connected"); return;
  }
  if (urlWeather.length() == 0) {
    Serial.println("Warning: updateTemp() urlWeather not present."); return;
  }

  HTTPClient http;
  http.begin(urlWeather);
  http.addHeader("Content-Type", "text/plain");

  int code = http.GET();
  if (code > 0) {
    String response = http.getString();
    Serial.println(code);
    Serial.println(response);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.println("updateTemp() Failed to parse JSON");
    } else {
      float temp     = doc["main"]["temp"]     | 0.0f;
      float humidity = doc["main"]["humidity"] | 0.0f;

      Serial.printf("Nhiet do: %.1f °C\r\n", temp);
      Serial.printf("Do am   : %.0f %%\r\n",  humidity);

      if (temp_ != temp) {
        temp_ = temp;
        Blynk.virtualWrite(V2, temp_);      // Nhiệt độ → V2
        Blynk.virtualWrite(V3, humidity);   // Độ ẩm   → V3
      }
    }
  } else {
    Serial.printf("updateTemp() HTTP error: %d\r\n", code);
  }
  http.end();
}

// Gửi IPv4 + Google Maps lên Blynk
void onceCalled() {
  static bool done_ = false;
  if (done_) return;
  done_ = true;

  String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longitude.c_str());

  Blynk.virtualWrite(V5, ip4Info.ip4.c_str());  // IPv4    → V5
  Blynk.virtualWrite(V6, link.c_str());          // Maps   → V6
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi " WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  // DNS cho Wokwi simulator
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(8, 8, 8, 8), IPAddress(8, 8, 4, 4));  // Google DNS to get it work

  Serial.print("local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}

void loop() {
  Blynk.run();

  currentMiliseconds = millis();
  onceCalled();    // Gửi IPv4 + Maps link lên Blynk (1 lần duy nhất)
  updateTemp();    // Cập nhật nhiệt độ / độ ẩm mỗi 10 giây
  uptimeBlynk();   // Uptime mỗi 1 giây
}

BLYNK_WRITE(V1) {
  int val = param.asInt();
  Serial.printf("[Blynk] V1 Den = %d\n", val);
}