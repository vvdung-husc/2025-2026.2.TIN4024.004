#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h> // Thư viện này chạy mượt hơn trên mô phỏng
#include <DHT.h>
#include <SPI.h>
#include <SD.h>

// WiFi Wokwi (Bắt buộc để chạy ảo)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SD_CS 5
AsyncWebServer server(80);

const unsigned long LOG_INTERVAL = 5000; 
unsigned long lastLogTime = 0;

// Hàm đọc dữ liệu từ SD để hiển thị
String getLogData() {
  File file = SD.open("/data.csv");
  if (!file) return "Chưa có dữ liệu";
  String content = "";
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();
  return content;
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Khởi tạo SD
  if (!SD.begin(SD_CS)) {
    Serial.println("❌ SD Card Error!");
  } else {
    Serial.println("✅ SD Card OK!");
    if (!SD.exists("/data.csv")) {
      File file = SD.open("/data.csv", FILE_WRITE);
      file.println("Time,Temperature,Humidity");
      file.close();
    }
  }

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected! IP: " + WiFi.localIP().toString());

  // Giao diện Web xịn xò
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    String html = "<html><head><meta charset='UTF-8'>";
    html += "<style>body{font-family:Arial; background:#eef2f3; text-align:center;}";
    html += ".card{background:white; padding:20px; border-radius:15px; display:inline-block; margin:10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1);}";
    html += ".btn{background:#007bff; color:white; padding:10px 20px; text-decoration:none; border-radius:5px;}</style></head><body>";
    html += "<h1>🌿 HỆ THỐNG GIÁM SÁT MÔI TRƯỜNG</h1>";
    html += "<div class='card'><h2>Nhiệt độ</h2><p style='font-size:2em; color:#ff4757;'>" + String(t,1) + "°C</p></div>";
    html += "<div class='card'><h2>Độ ẩm</h2><p style='font-size:2em; color:#2ed573;'>" + String(h,1) + "%</p></div><br><br>";
    html += "<a href='/download' class='btn'>Tải File Log CSV để phân tích</a>";
    html += "<h3>Dữ liệu thô từ SD Card:</h3><pre style='background:#fff; padding:10px;'>" + getLogData() + "</pre>";
    html += "</body></html>";
    
    request->send(200, "text/html", html);
  });

  // Đường dẫn tải file
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/data.csv", "text/csv");
  });

  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = currentMillis;
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      File file = SD.open("/data.csv", FILE_APPEND);
      if (file) {
        // Ghi định dạng CSV chuẩn: Thời gian, Nhiệt độ, Độ ẩm
        file.printf("%lu,%.2f,%.2f\n", currentMillis/1000, t, h);
        file.close();
        Serial.println("Logged: " + String(t) + "C");
      }
    }
  }
}