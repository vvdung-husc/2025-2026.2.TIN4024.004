#include <Arduino.h>

// Định nghĩa chân cắm dựa theo hình ảnh diagram.json
#define PIN_LED_RED     23
#define PIN_LED_YELLOW  22
#define PIN_LED_GREEN   21

enum TrafficState { STATE_GREEN, STATE_YELLOW, STATE_RED };

unsigned long stateTimer = 0;  // Quản lý thời gian chuyển màu
unsigned long blinkTimer = 0;  // Quản lý nhịp nhấp nháy
bool blinkState = LOW;         // Trạng thái Bật/Tắt khi nhấp nháy
TrafficState currentState = STATE_GREEN;

// Hàm cập nhật trạng thái vật lý của đèn
void updatePhysicalLeds(bool isVisible) {
  // Tắt tất cả các đèn trước khi cập nhật
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  // Nếu trong nhịp "Bật" (isVisible = true), đèn tương ứng với trạng thái sẽ sáng
  if (isVisible) {
    switch (currentState) {
      case STATE_GREEN:  digitalWrite(PIN_LED_GREEN,  HIGH); break;
      case STATE_YELLOW: digitalWrite(PIN_LED_YELLOW, HIGH); break;
      case STATE_RED:    digitalWrite(PIN_LED_RED,    HIGH); break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  printf("HỆ THỐNG ĐÈN GIAO THÔNG NHẤP NHÁY\n");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  stateTimer = millis();
  blinkTimer = millis();
}

void loop() {
  unsigned long now = millis();

  // 1. XỬ LÝ NHỊP NHẤP NHÁY (Chu kỳ 500ms)
  if (now - blinkTimer >= 500) {
    blinkState = !blinkState; 
    blinkTimer = now;
    updatePhysicalLeds(blinkState);
  }

  // 2. XỬ LÝ CHUYỂN TRẠNG THÁI (Logic thời gian 7-3-5)
  switch (currentState) {
    case STATE_GREEN:
      if (now - stateTimer >= 7000) {
        printf("[XANH] 7s kết thúc -> Sang VÀNG\n");
        currentState = STATE_YELLOW;
        stateTimer = now;
      }
      break;

    case STATE_YELLOW:
      if (now - stateTimer >= 3000) {
        printf("[VÀNG] 3s kết thúc -> Sang ĐỎ\n");
        currentState = STATE_RED;
        stateTimer = now;
      }
      break;

    case STATE_RED:
      if (now - stateTimer >= 5000) {
        printf("[ĐỎ  ] 5s kết thúc -> Sang XANH\n");
        currentState = STATE_GREEN;
        stateTimer = now;
      }
      break;
  }
}