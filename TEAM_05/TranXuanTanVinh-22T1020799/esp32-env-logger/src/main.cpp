#include <Arduino.h>
/*
 * HỆ THỐNG GHI NHẬT KÝ MÔI TRƯỜNG VỚI ESP32
 * =============================================
 * Wokwi Simulation — DHT22 + LCD I2C + SD Card + ThingSpeak
 * 
 * QUAN TRỌNG: Dùng thư viện DHTesp (hoạt động ổn trên Wokwi ESP32)
 *             Dùng thư viện ThingSpeak (kết nối ThingSpeak đúng cách)
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include "DHTesp.h"
#include "ThingSpeak.h"

// ==================== CẤU HÌNH ====================

// WiFi
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// ThingSpeak — THAY BẰNG THÔNG TIN CHANNEL CỦA BẠN
unsigned long CHANNEL_ID       = 3324632;        // ← Thay Channel ID
const char*  WRITE_API_KEY     = "QVJVQ7D0QAP5P9IZ";  // ← Thay Write API Key

// Chân kết nối
#define DHT_PIN       15      // DHT22 data
#define SD_CS_PIN     5       // SD Card CS
#define LED_GREEN     2       // LED xanh (trạng thái OK)
#define LED_RED       4       // LED đỏ (cảnh báo) — dùng GPIO4 thay vì 15
#define BUZZER_PIN    13      // Buzzer

// Ngưỡng cảnh báo
#define TEMP_HIGH     40.0
#define TEMP_LOW      10.0
#define HUMID_HIGH    80.0
#define HUMID_LOW     20.0

// Chu kỳ
#define READ_INTERVAL   3000    // Đọc mỗi 3 giây
#define SEND_INTERVAL   15000   // Gửi ThingSpeak mỗi 15 giây

// ==================== KHỞI TẠO ĐỐI TƯỢNG ====================

DHTesp dhtSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient client;

// Biến toàn cục
float temperature = 0;
float humidity = 0;
bool sdReady = false;
bool wifiConnected = false;
unsigned long lastReadTime = 0;
unsigned long lastSendTime = 0;
unsigned long readingCount = 0;

// Ký tự tùy chỉnh LCD
byte thermIcon[8] = { B00100, B01010, B01010, B01110, B01110, B11111, B11111, B01110 };
byte dropIcon[8]  = { B00100, B00100, B01010, B01010, B10001, B10001, B10001, B01110 };
byte wifiIcon[8]  = { B00000, B01110, B10001, B00100, B01010, B00000, B00100, B00000 };
byte sdIconCh[8]  = { B01110, B10001, B11111, B10001, B10001, B10001, B11111, B00000 };

// ==================== FORWARD DECLARATIONS ====================
void beep(int duration);
void initSD();
void connectWiFi();
void readSensor();
void updateLCD();
void logToSD();
void checkAlerts();
void sendToThingSpeak();

// ==================== SETUP ====================

void setup() {
  Serial.begin(115200);
  Serial.println("\n========================================");
  Serial.println("  HE THONG GHI NHAT KY MOI TRUONG");
  Serial.println("  ESP32 + DHT22 + SD + ThingSpeak");
  Serial.println("========================================\n");

  // GPIO
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  // Test LED khi khởi động
  Serial.println("[TEST] Nhap nhay LED...");
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, HIGH);
    delay(200);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    delay(200);
  }

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, thermIcon);
  lcd.createChar(1, dropIcon);
  lcd.createChar(2, wifiIcon);
  lcd.createChar(3, sdIconCh);

  lcd.setCursor(1, 0);
  lcd.print("ENV LOGGER v2");
  lcd.setCursor(1, 1);
  lcd.print("Khoi dong...");
  delay(1500);

  // DHT22 — dùng DHTesp (ổn định trên Wokwi ESP32)
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  Serial.println("[OK] DHT22 da khoi tao (DHTesp)");

  // SD Card
  initSD();

  // WiFi
  connectWiFi();

  // ThingSpeak
  if (wifiConnected) {
    ThingSpeak.begin(client);
    Serial.println("[OK] ThingSpeak da khoi tao");
  }

  // Beep sẵn sàng
  beep(100);
  delay(100);
  beep(100);

  lcd.clear();
  Serial.println("\n[OK] He thong san sang!\n");
}

// ==================== LOOP ====================

void loop() {
  unsigned long now = millis();

  // Đọc cảm biến
  if (now - lastReadTime >= READ_INTERVAL) {
    lastReadTime = now;
    readSensor();
    updateLCD();
    logToSD();
    checkAlerts();
  }

  // Gửi ThingSpeak
  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;
    sendToThingSpeak();
  }

  // LED xanh nhấp nháy = hệ thống đang chạy
  digitalWrite(LED_GREEN, (millis() / 500) % 2);
}

// ==================== BEEP ====================

void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

// ==================== ĐỌC CẢM BIẾN ====================

void readSensor() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("[LOI] Khong doc duoc DHT22!");
    digitalWrite(LED_RED, HIGH);
    return;
  }

  digitalWrite(LED_RED, LOW);
  temperature = data.temperature;
  humidity = data.humidity;
  readingCount++;

  Serial.printf("[#%lu] Nhiet do: %.1f°C | Do am: %.1f%%\n",
                readingCount, temperature, humidity);
}

// ==================== LCD ====================

void updateLCD() {
  // Dòng 1: Nhiệt độ + icon trạng thái
  lcd.setCursor(0, 0);
  lcd.write(0);  // thermometer
  lcd.print(" ");
  lcd.print(temperature, 1);
  lcd.print((char)223);  // ký tự °
  lcd.print("C   ");

  lcd.setCursor(13, 0);
  if (wifiConnected) lcd.write(2);  // wifi icon
  else lcd.print(" ");

  lcd.setCursor(15, 0);
  if (sdReady) lcd.write(3);  // sd icon
  else lcd.print("!");

  // Dòng 2: Độ ẩm
  lcd.setCursor(0, 1);
  lcd.write(1);  // drop
  lcd.print(" ");
  lcd.print(humidity, 1);
  lcd.print("%  #");
  lcd.print(readingCount);
  lcd.print("  ");
}

// ==================== SD CARD ====================

void initSD() {
  Serial.print("[...] Khoi tao the SD...");

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(" THAT BAI!");
    sdReady = false;
    return;
  }

  sdReady = true;
  Serial.println(" OK!");

  if (!SD.exists("/env_log.csv")) {
    File f = SD.open("/env_log.csv", FILE_WRITE);
    if (f) {
      f.println("STT,Thoi_gian_ms,Nhiet_do_C,Do_am_phan_tram,Trang_thai");
      f.close();
      Serial.println("[OK] Tao file env_log.csv");
    }
  }
}

void logToSD() {
  if (!sdReady) return;

  File f = SD.open("/env_log.csv", FILE_APPEND);
  if (!f) {
    Serial.println("[LOI] Khong mo duoc file SD!");
    return;
  }

  String status = "OK";
  if (temperature > TEMP_HIGH || humidity > HUMID_HIGH) status = "CANH_BAO_CAO";
  else if (temperature < TEMP_LOW || humidity < HUMID_LOW) status = "CANH_BAO_THAP";

  f.printf("%lu,%lu,%.1f,%.1f,%s\n",
           readingCount, millis(), temperature, humidity, status.c_str());
  f.close();

  Serial.printf("[SD] Ghi ban ghi #%lu\n", readingCount);
}

// ==================== WIFI ====================

void connectWiFi() {
  Serial.printf("[...] Ket noi WiFi: %s", WIFI_SSID);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ket noi WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(attempts % 16, 1);
    lcd.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.printf("\n[OK] IP: %s\n", WiFi.localIP().toString().c_str());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    // Bật LED xanh xác nhận
    digitalWrite(LED_GREEN, HIGH);
    delay(1500);
  } else {
    wifiConnected = false;
    Serial.println("\n[LOI] WiFi THAT BAI!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi THAT BAI");
    lcd.setCursor(0, 1);
    lcd.print("Chi ghi SD...");
    // Bật LED đỏ báo lỗi
    digitalWrite(LED_RED, HIGH);
    delay(1500);
    digitalWrite(LED_RED, LOW);
  }
}

// ==================== THINGSPEAK ====================

void sendToThingSpeak() {
  if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
    Serial.println("[ThingSpeak] Khong co WiFi, bo qua.");
    return;
  }

  // Đặt dữ liệu vào các field
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);

  // Gửi tất cả field cùng lúc
  Serial.print("[ThingSpeak] Dang gui...");
  int result = ThingSpeak.writeFields(CHANNEL_ID, WRITE_API_KEY);

  if (result == 200) {
    Serial.println(" THANH CONG! (200 OK)");
    // Nhấp nháy LED xanh xác nhận
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_GREEN, HIGH);
      delay(80);
      digitalWrite(LED_GREEN, LOW);
      delay(80);
    }
  } else {
    Serial.printf(" LOI! Ma loi: %d\n", result);
    digitalWrite(LED_RED, HIGH);
    delay(300);
    digitalWrite(LED_RED, LOW);
  }
}

// ==================== CẢNH BÁO ====================

void checkAlerts() {
  bool alert = false;

  if (temperature > TEMP_HIGH) {
    Serial.println("[!!!] CANH BAO: Nhiet do QUA CAO!");
    alert = true;
  }
  if (temperature < TEMP_LOW) {
    Serial.println("[!!!] CANH BAO: Nhiet do QUA THAP!");
    alert = true;
  }
  if (humidity > HUMID_HIGH) {
    Serial.println("[!!!] CANH BAO: Do am QUA CAO!");
    alert = true;
  }
  if (humidity < HUMID_LOW) {
    Serial.println("[!!!] CANH BAO: Do am QUA THAP!");
    alert = true;
  }

  if (alert) {
    digitalWrite(LED_RED, HIGH);
    beep(500);
    delay(200);
    beep(500);
    delay(200);
    digitalWrite(LED_RED, LOW);
  }
}
