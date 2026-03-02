#include <Arduino.h>
#include <TM1637Display.h>

const int RED_PIN = 18;
const int YELLOW_PIN = 5;
const int GREEN_PIN = 17;
const int BLUE_LED_PIN = 12; // Đèn liên kết với nút bấm
const int BUTTON_PIN = 13;
const int LDR_PIN = 34;

// Khởi tạo màn hình TM1637 (CLK: 22, DIO: 23)
TM1637Display display(22, 23);

// Biến trạng thái
bool displayEnabled = true; 
int state = 0;              // 0: Xanh, 1: Vàng, 2: Đỏ
unsigned long lastTick = 0;
int timeLeft = 7;           // Bắt đầu với đèn Xanh 7 giây
bool lastBtnState = HIGH;

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  display.setBrightness(7); // Độ sáng tối đa
  Serial.begin(115200);
}

void loop() {
  // 1. Xử lý Nút bấm để Bật/Tắt bảng đếm ngược
  bool currentBtn = digitalRead(BUTTON_PIN);
  if (lastBtnState == HIGH && currentBtn == LOW) {
    displayEnabled = !displayEnabled;
    delay(50); // Chống rung nút
  }
  lastBtnState = currentBtn;

  // Đèn Blue sáng khi bảng đếm ngược đang ở trạng thái Bật
  digitalWrite(BLUE_LED_PIN, displayEnabled ? HIGH : LOW);

  // 2. Đọc cảm biến ánh sáng (LDR)
  int lightLevel = analogRead(LDR_PIN);

  if (lightLevel > 3000) { 
    // CHẾ ĐỘ TRỜI TỐI: Vàng nhấp nháy, tắt các đèn khác và bảng số
    runNightMode();
  } else {
    // CHẾ ĐỘ BÌNH THƯỜNG: Đèn giao thông chạy và đếm ngược
    runTrafficCycle();
  }
}

void runTrafficCycle() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastTick >= 1000) {
    lastTick = currentMillis;
    
    // Cập nhật đèn LED theo trạng thái hiện tại
    digitalWrite(GREEN_PIN, state == 0);
    digitalWrite(YELLOW_PIN, state == 1);
    digitalWrite(RED_PIN, state == 2);

    // Cập nhật bảng đếm ngược nếu được phép bật
    if (displayEnabled) {
      display.showNumberDec(timeLeft, true, 2, 2); // Luôn hiện 2 chữ số (ví dụ: 07, 02)
    } else {
      display.clear();
    }

    // Trừ thời gian
    timeLeft--;

    // Chuyển trạng thái khi hết thời gian
    if (timeLeft < 0) {
      state = (state + 1) % 3;
      if (state == 0) timeLeft = 7;   // Xanh 7s
      else if (state == 1) timeLeft = 3;  // Vàng 3s
      else if (state == 2) timeLeft = 10; // Đỏ 10s
    }
  }
}

void runNightMode() {
  // Tắt các đèn không liên quan
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  display.clear();
  
  // Đèn vàng nhấp nháy chu kỳ 1 giây (500ms sáng / 500ms tắt)
  digitalWrite(YELLOW_PIN, (millis() / 500) % 2);
}