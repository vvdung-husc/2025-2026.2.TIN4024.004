#include <Arduino.h>
#include <TM1637Display.h>

// ===== GPIO =====
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    21
#define BUTTON_PIN  23

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

// ===== Biến trạng thái =====
bool blueLedState = false;
bool lastButtonState = HIGH;

// ===== Hàm xử lý nút bấm =====
void handleButton() {
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW) {
    blueLedState = !blueLedState;
    digitalWrite(LED_BLUE, blueLedState);
    delay(200); // chống dội
  }

  lastButtonState = buttonState;
}

// ===== Bật 1 đèn, tắt đèn còn lại =====
void setTrafficLight(bool red, bool yellow, bool green) {
  digitalWrite(LED_RED, red);
  digitalWrite(LED_YELLOW, yellow);
  digitalWrite(LED_GREEN, green);
}

// ===== Đếm ngược + đọc nút mỗi giây =====
void countdown(int seconds) {
  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i, true);

    // chia nhỏ 1 giây để còn đọc nút
    for (int j = 0; j < 10; j++) {
      handleButton();
      delay(100);
    }
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.clear();
}

void loop() {
  // 🔴 Đèn đỏ 5s
  setTrafficLight(true, false, false);
  countdown(5);

  // 🟢 Đèn xanh 7s
  setTrafficLight(false, false, true);
  countdown(7);

  // 🟡 Đèn vàng 3s
  setTrafficLight(false, true, false);
  countdown(3);
}
