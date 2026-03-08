#define BLYNK_TEMPLATE_ID "TMPL6b74AeMpm"
#define BLYNK_TEMPLATE_NAME "blynk"
#define BLYNK_AUTH_TOKEN "KX8veL_SkLx1pGEQJ-iP80Q9ISiGtXDM"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

#define DHTPIN 16          // Chân Data của DHT22
#define DHTTYPE DHT22
#define CLK 18             
#define DIO 19             
#define PIN_LED 21         // Đèn xanh
#define PIN_BUTTON 23      // Nút bấm vật lý màu xanh trên Wokwi

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

// --- Biến trạng thái hệ thống ---
bool isSystemOn = true;    // Quản lý trạng thái "Đèn xanh, bảng đếm ngược"
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// --- Hàm 1: Cập nhật thời gian hoạt động (Mỗi 1 giây) ---
void updateUptime() {
  if (!isSystemOn) return; // Nếu hệ thống OFF, ngừng đếm và hiện số
  
  long uptime = millis() / 1000;
  Blynk.virtualWrite(V0, uptime); 
  display.showNumberDec(uptime); 
}

// --- Hàm 2: Gửi dữ liệu cảm biến (Mỗi 2 giây) ---
void sendSensorData() {
  if (!isSystemOn) return; // Nếu hệ thống OFF, ngừng gửi dữ liệu cảm biến

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) return;

  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);
}

// --- Hàm 3: Kiểm tra nút bấm vật lý chân 23 (Liên tục mỗi 0.1s) ---
void checkPhysicalButton() {
  // Đọc tín hiệu từ nút bấm vật lý trên Wokwi
  if (digitalRead(PIN_BUTTON) == LOW) {
    delay(200); // Khử nhiễu nút bấm
    isSystemOn = !isSystemOn; // Đảo trạng thái hệ thống
    
    // Cập nhật trạng thái ngược lại lên nút bấm Blynk (V1) để đồng bộ
    Blynk.virtualWrite(V1, isSystemOn);
    
    // Thực hiện hành động Bật/Tắt ngay lập tức
    if (!isSystemOn) {
      display.clear();            // Tắt màn hình
      digitalWrite(PIN_LED, LOW); // Tắt đèn
    } else {
      digitalWrite(PIN_LED, HIGH); // Bật đèn
    }
    
    Serial.print("Nút Wokwi nhấn! Trạng thái hệ thống: ");
    Serial.println(isSystemOn ? "ON" : "OFF");
    while(digitalRead(PIN_BUTTON) == LOW); // Đợi nhả nút
  }
}

// --- Hàm 4: Nhận lệnh từ nút bấm ảo trên Blynk (V1) ---
BLYNK_WRITE(V1) {
  isSystemOn = param.asInt(); // Nhận 0 hoặc 1 từ nút gạt Blynk
  
  if (!isSystemOn) {
    display.clear();            // Tắt bảng đếm ngược
    digitalWrite(PIN_LED, LOW); // Tắt đèn xanh
    Serial.println("Blynk OFF: Dừng hệ thống.");
  } else {
    digitalWrite(PIN_LED, HIGH); // Bật đèn xanh
    Serial.println("Blynk ON: Khởi động hệ thống.");
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP); // Kích hoạt trở kéo lên cho nút bấm vật lý
  
  digitalWrite(PIN_LED, HIGH); // Mặc định ban đầu đèn sáng
  dht.begin();
  display.setBrightness(0x0f);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "128.199.144.129", 80);

  // Thiết lập các Timer chạy định kỳ
  timer.setInterval(100L, checkPhysicalButton); // Kiểm tra nút bấm vật lý cực nhanh
  timer.setInterval(1000L, updateUptime);       // Cập nhật giây và màn hình mỗi 1s
  timer.setInterval(2000L, sendSensorData);    // Đọc DHT22 mỗi 2s
}

void loop() {
  Blynk.run();
  timer.run();
}