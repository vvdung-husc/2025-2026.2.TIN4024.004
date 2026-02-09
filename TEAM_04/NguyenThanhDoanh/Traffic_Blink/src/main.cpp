#include <Arduino.h>

// Định nghĩa các chân cắm (GPIO)
#define PIN_LED_RED    25
#define PIN_LED_YELLOW 26
#define PIN_LED_GREEN  32

// Định nghĩa các trạng thái đèn
enum TrafficState { RED, YELLOW, GREEN };
TrafficState currentState = RED;

unsigned long previousMillis = 0;

// Cấu hình thời gian (mili giây)
const long intervalRed    = 5000;
const long intervalYellow = 3000;
const long intervalGreen  = 7000;

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  // Khởi tạo trạng thái ban đầu
  digitalWrite(PIN_LED_RED, HIGH);
  Serial.println("LED [RED   ] ON => 5 Seconds");
}

void loop() {
  unsigned long currentMillis = millis();
  long currentInterval = 0;

  // Xác định khoảng thời gian chờ dựa trên trạng thái hiện tại
  switch (currentState) {
    case RED:    currentInterval = intervalRed;    break;
    case YELLOW: currentInterval = intervalYellow; break;
    case GREEN:  currentInterval = intervalGreen;  break;
  }

  // Kiểm tra nếu đã hết thời gian chờ
  if (currentMillis - previousMillis >= currentInterval) {
    previousMillis = currentMillis; // Lưu lại thời điểm chuyển trạng thái

    // Tắt tất cả các đèn trước khi bật đèn mới
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);

    // Chuyển sang trạng thái tiếp theo
    if (currentState == RED) {
      currentState = GREEN;
      digitalWrite(PIN_LED_GREEN, HIGH);
      Serial.println("LED [GREEN ] ON => 7 Seconds");
    } 
    else if (currentState == GREEN) {
      currentState = YELLOW;
      digitalWrite(PIN_LED_YELLOW, HIGH);
      Serial.println("LED [YELLOW] ON => 3 Seconds");
    } 
    else if (currentState == YELLOW) {
      currentState = RED;
      digitalWrite(PIN_LED_RED, HIGH);
      Serial.println("LED [RED   ] ON => 5 Seconds");
    }
  }
}