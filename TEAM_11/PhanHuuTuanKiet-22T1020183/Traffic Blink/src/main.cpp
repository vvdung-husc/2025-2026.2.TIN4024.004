#include <Arduino.h>

// ====== KHAI BÁO CHÂN LED ======
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

// ====== THỜI GIAN SÁNG (đơn vị: milliseconds) ======
#define TIME_RED     5000    // Đèn đỏ sáng 5 giây
#define TIME_YELLOW  3000    // Đèn vàng sáng 3 giây
#define TIME_GREEN   7000    // Đèn xanh sáng 7 giây

// ====== CÁC TRẠNG THÁI ĐÈN ======
enum TrafficState {
  RED,        // Trạng thái đèn đỏ
  YELLOW,     // Trạng thái đèn vàng
  GREEN       // Trạng thái đèn xanh
};
// Trạng thái hiện tại (bắt đầu bằng đèn đỏ)
TrafficState currentState = RED;

// Biến lưu thời điểm trước đó (dùng cho millis)
unsigned long previousMillis = 0;

// ====== HÀM TẮT TOÀN BỘ LED ======
void turnOffAllLEDs() {
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
}
void setup() {
  Serial.begin(115200);
  // Cấu hình chân LED là OUTPUT
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  // Tắt toàn bộ LED ban đầu
  turnOffAllLEDs();
  // Bật đèn đỏ đầu tiên
  digitalWrite(PIN_LED_RED, HIGH);
  Serial.println("LED [RED] ON => 5 Seconds");
}
void loop() {
  // Lấy thời gian hiện tại (tính từ lúc board chạy)
  unsigned long currentMillis = millis();

  // Kiểm tra trạng thái hiện tại
  switch (currentState) {
    // ====== TRẠNG THÁI ĐÈN ĐỎ ======
    case RED:
      // Nếu đã đủ thời gian đèn đỏ
      if (currentMillis - previousMillis >= TIME_RED) {
        previousMillis = currentMillis; // cập nhật mốc thời gian
        turnOffAllLEDs();               // tắt tất cả LED

        digitalWrite(PIN_LED_YELLOW, HIGH); // bật đèn vàng
        Serial.println("LED [YELLOW] ON => 3 Seconds");

        currentState = YELLOW; // chuyển sang trạng thái vàng
      }
      break;
    // ====== TRẠNG THÁI ĐÈN VÀNG ======
    case YELLOW:
      if (currentMillis - previousMillis >= TIME_YELLOW) {
        previousMillis = currentMillis;
        turnOffAllLEDs();
        digitalWrite(PIN_LED_GREEN, HIGH); // bật đèn xanh
        Serial.println("LED [GREEN] ON => 7 Seconds");

        currentState = GREEN;
      }
      break;
    // ====== TRẠNG THÁI ĐÈN XANH ======
    case GREEN:
      if (currentMillis - previousMillis >= TIME_GREEN) {
        previousMillis = currentMillis;
        turnOffAllLEDs();

        digitalWrite(PIN_LED_RED, HIGH); // quay lại đèn đỏ
        Serial.println("LED [RED] ON => 5 Seconds");

        currentState = RED;
      }
      break;
  }
}
