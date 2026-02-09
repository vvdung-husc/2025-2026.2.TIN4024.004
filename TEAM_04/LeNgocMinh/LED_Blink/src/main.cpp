#include <Arduino.h>

// Định nghĩa các chân cắm
#define PIN_LED_RED 23
#define PIN_LED_YELLOW 22
#define PIN_LED_GREEN 21

// Hàm kiểm tra thời gian Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  Serial.begin(115200); // Khởi tạo Serial để printf hoạt động
  printf("TRAFFIC LIGHT NON-BLOCKING READY\n");
  
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
}

void loop() {
  // Biến lưu thời gian và trạng thái cho từng đèn
  static unsigned long timerRed = 0;
  static unsigned long timerYellow = 0;
  static unsigned long timerGreen = 0;

  static bool statusRed = false;
  static bool statusYellow = false;
  static bool statusGreen = false;

  // Điều khiển Đèn Đỏ (Chớp tắt mỗi 500ms)
  if (IsReady(timerRed, 500)) {
    statusRed = !statusRed;
    digitalWrite(PIN_LED_RED, statusRed);
    printf("RED LED: %s\n", statusRed ? "ON" : "OFF");
  }

  // Điều khiển Đèn Vàng (Chớp tắt mỗi 1000ms - chậm hơn)
  if (IsReady(timerYellow, 1000)) {
    statusYellow = !statusYellow;
    digitalWrite(PIN_LED_YELLOW, statusYellow);
    printf("YELLOW LED: %s\n", statusYellow ? "ON" : "OFF");
  }

  // Điều khiển Đèn Xanh (Chớp tắt mỗi 1500ms - chậm nhất)
  if (IsReady(timerGreen, 1500)) {
    statusGreen = !statusGreen;
    digitalWrite(PIN_LED_GREEN, statusGreen);
    printf("GREEN LED: %s\n", statusGreen ? "ON" : "OFF");
  }
}