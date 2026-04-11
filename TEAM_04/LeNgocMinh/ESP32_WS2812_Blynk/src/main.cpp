/* ==========================================================
 * DỰ ÁN: HỆ THỐNG ĐÈN LED THÔNG MINH RGB WS2812B
 * PHIÊN BẢN: Đầy đủ nhất - App (zeRGBa), Web (Slider) & Status (V6)
 * ========================================================== */

#define BLYNK_TEMPLATE_ID "TMPL6tXpSXJJi"
#define BLYNK_TEMPLATE_NAME "Smart LED RGB"
#define BLYNK_AUTH_TOKEN "co3fl3-u6X2tQCFbdb1lgOyJPnd2D8D1"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_NeoPixel.h>

/* --- CẤU HÌNH PHẦN CỨNG --- */
#define LED_PIN 16        
#define NUM_LEDS 16       
#define BTN_PIN 14        

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
BlynkTimer timer;

// Biến lưu trạng thái màu sắc và độ sáng
int currentBrightness = 255;
bool isPowerOn = true;
int r = 255, g = 255, b = 255; 

/* --- HÀM CẬP NHẬT ĐÈN LED --- */
void updateLEDs() {
  if (isPowerOn) {
    strip.setBrightness(currentBrightness); 
    strip.fill(strip.Color(r, g, b));
  } else {
    strip.clear();
  }
  strip.show();
}

/* --- HÀM GỬI THÔNG BÁO MÀU LÊN WEB DASHBOARD (V6) --- */
void sendColorStatusToWeb() {
  char colorMsg[32];
  sprintf(colorMsg, "RGB: (%d, %d, %d)", r, g, b);
  Blynk.virtualWrite(V0, colorMsg);

  Serial.print("Cập nhật trạng thái - ");
  Serial.println(colorMsg);
}

/* ==========================================================
 * KHỐI NHẬN LỆNH TỪ BLYNK (APP & WEB)
 * ========================================================== */

// V0: BẢNG MÀU zeRGBa (Dùng trên App điện thoại - Chế độ Merge)
BLYNK_WRITE(V0) {
  r = param[0].asInt(); 
  g = param[1].asInt();
  b = param[2].asInt();
  updateLEDs();
  
  // Cập nhật ngược lại các Slider trên Web để đồng bộ
  Blynk.virtualWrite(V3, r);
  Blynk.virtualWrite(V4, g);
  Blynk.virtualWrite(V5, b);
  
  sendColorStatusToWeb(); // Gửi thông báo màu mới lên Web (Widget Labeled Value V6)
  Serial.printf("Màu từ zeRGBa (V0): R:%d G:%d B:%d\n", r, g, b);
}

// V1: ĐỘ SÁNG (Slider trên cả App và Web)
BLYNK_WRITE(V1) {
  currentBrightness = param.asInt();
  updateLEDs();
  Serial.printf("Độ sáng mới: %d\n", currentBrightness);
}

// V2: NÚT NGUỒN (ON/OFF - Dùng cho Web, App và Google Assistant)
BLYNK_WRITE(V2) {
  isPowerOn = param.asInt();
  updateLEDs();
  Serial.println(isPowerOn ? "Thiết bị: BẬT" : "Thiết bị: TẮT");
}

// V3, V4, V5: 3 SLIDER RIÊNG BIỆT (Dùng trên Web Dashboard)
BLYNK_WRITE(V3) { r = param.asInt(); updateLEDs(); sendColorStatusToWeb(); }
BLYNK_WRITE(V4) { g = param.asInt(); updateLEDs(); sendColorStatusToWeb(); }
BLYNK_WRITE(V5) { b = param.asInt(); updateLEDs(); sendColorStatusToWeb(); }

/* ==========================================================
 * KHỐI ĐIỀU KHIỂN VẬT LÝ (NÚT NHẤN)
 * ========================================================== */
void checkPhysicalButton() {
  if (digitalRead(BTN_PIN) == LOW) { 
    isPowerOn = !isPowerOn; 
    Blynk.virtualWrite(V2, isPowerOn); // Đồng bộ nút nhấn lên App/Web
    updateLEDs();
    Serial.println("Nút nhấn vật lý: Đã đảo trạng thái Power");
    delay(300); 
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  strip.begin();
  strip.setBrightness(currentBrightness);
  updateLEDs(); 
  
  Serial.println("Đang kết nối Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Gửi trạng thái màu mặc định lên Web ngay khi hệ thống vừa kết nối thành công
  sendColorStatusToWeb();
  
  timer.setInterval(100L, checkPhysicalButton);
}

void loop() {
  Blynk.run();
  timer.run();
}