#define BLYNK_TEMPLATE_ID "TMPL6WGb1N3i2"
#define BLYNK_TEMPLATE_NAME "ESP32 IOT"
#define BLYNK_AUTH_TOKEN "6Dd6oR8RIIjMcpX23bke5sWWrLJXBtLC"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include <Wire.h>                 // Thư viện giao tiếp I2C
#include <LiquidCrystal_I2C.h>    // Thư viện điều khiển LCD

#define DHTPIN 4
#define DHTTYPE DHT22
#define LEDPIN 2
#define RELAYPIN 5                // Thêm khai báo chân Relay

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Khởi tạo LCD địa chỉ 0x27

// --- ĐIỀU KHIỂN LED VÀ RELAY TỪ BLYNK ---
BLYNK_WRITE(V2) {
  int state = param.asInt();
  digitalWrite(LEDPIN, state);
  digitalWrite(RELAYPIN, state);    // Relay sẽ bật/tắt cùng lúc với LED
}

// --- ĐỌC CẢM BIẾN VÀ HIỂN THỊ ---
void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT read failed");
    return;
  }

  // 1. Gửi dữ liệu lên app Blynk
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  // 2. In ra Serial Monitor
  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print(" | Hum: ");
  Serial.println(h);

  // 3. Hiển thị lên màn hình LCD
  lcd.setCursor(0, 0); // Dòng 1
  lcd.print("Temp: ");
  lcd.print(t, 1);
  lcd.print(" C  ");
  
  lcd.setCursor(0, 1); // Dòng 2
  lcd.print("Hum:  ");
  lcd.print(h, 1);
  lcd.print(" %  ");
}

void setup() {
  Serial.begin(115200);

  // Cấu hình chân output
  pinMode(LEDPIN, OUTPUT);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  digitalWrite(RELAYPIN, LOW); // Ban đầu tắt Relay và LED

  // Cấu hình màn hình LCD
  Wire.begin(21, 22);          // Ép ESP32 dùng đúng chân I2C đã nối
  lcd.init();
  lcd.backlight();             // Bật đèn nền LCD
  lcd.setCursor(0, 0);
  lcd.print("Dang ket noi..."); // Báo hiệu đang kết nối WiFi

  dht.begin();

  // Kết nối WiFi và Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connectWiFi(ssid, pass);

  lcd.clear(); // Xóa chữ "Dang ket noi..." khi kết nối thành công

  // Hẹn giờ đọc cảm biến 2 giây/lần
  timer.setInterval(2000L, sendSensor);
}

void loop() {
  Blynk.run();
  timer.run();
}
