#include <Arduino.h>

// 1. Định nghĩa chân
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

// 2. Định nghĩa trạng thái
#define STATE_RED     0
#define STATE_YELLOW  1
#define STATE_GREEN   2

// 3. Biến toàn cục
int currentState = STATE_RED;
int ledState = LOW;               // Trạng thái bật/tắt của nhịp nháy (LOW hoặc HIGH)

// --- HAI BIẾN THỜI GIAN RIÊNG BIỆT ---
unsigned long timeForStateChange = 0; // Mốc thời gian để chuyển màu (Đỏ->Vàng->Xanh)
unsigned long timeForBlink = 0;       // Mốc thời gian để chớp tắt (Nhấp nháy)

unsigned long stateInterval = 5000;   // Thời gian giữ màu (mặc định Đỏ 5s)
const long blinkSpeed = 200;          // Tốc độ nhấp nháy (200ms = nhanh)

void setup() {
  Serial.begin(115200);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  Serial.println("Start: RED Blinking");
}

void loop() {
  unsigned long currentMillis = millis();

  // --- NHIỆM VỤ 1: XỬ LÝ NHẤP NHÁY (BLINK) ---
  // Cứ mỗi 200ms thì đảo trạng thái đèn (Sáng -> Tắt -> Sáng...)
  if (currentMillis - timeForBlink >= blinkSpeed) {
    timeForBlink = currentMillis; // Cập nhật lại đồng hồ blink
    
    // Đảo ngược trạng thái: Nếu đang tắt thì bật, đang bật thì tắt
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // Áp dụng trạng thái nhấp nháy cho đúng cái đèn đang được chọn
    switch (currentState) {
      case STATE_RED:
        digitalWrite(LED_RED, ledState);   // Nháy đỏ
        digitalWrite(LED_YELLOW, LOW);     // Vàng luôn tắt
        digitalWrite(LED_GREEN, LOW);      // Xanh luôn tắt
        break;
      case STATE_YELLOW:
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, ledState); // Nháy vàng
        digitalWrite(LED_GREEN, LOW);
        break;
      case STATE_GREEN:
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_GREEN, ledState);  // Nháy xanh
        break;
    }
  }

  // --- NHIỆM VỤ 2: XỬ LÝ CHUYỂN MÀU (STATE CHANGE) ---
  // Kiểm tra xem đã hết thời gian của màu hiện tại chưa (5s, 3s hoặc 7s)
  if (currentMillis - timeForStateChange >= stateInterval) {
    timeForStateChange = currentMillis; // Cập nhật lại đồng hồ chuyển màu

    // Chuyển sang màu tiếp theo
    switch (currentState) {
      case STATE_RED:
        currentState = STATE_YELLOW;
        stateInterval = 3000; // Vàng nháy trong 3 giây
        Serial.println("Switching to YELLOW (3s)");
        // Quan trọng: Tắt hẳn đèn cũ để tránh bị lưu trạng thái đang sáng
        digitalWrite(LED_RED, LOW); 
        break;

      case STATE_YELLOW:
        currentState = STATE_GREEN;
        stateInterval = 7000; // Xanh nháy trong 7 giây
        Serial.println("Switching to GREEN (7s)");
        digitalWrite(LED_YELLOW, LOW);
        break;

      case STATE_GREEN:
        currentState = STATE_RED;
        stateInterval = 5000; // Đỏ nháy trong 5 giây
        Serial.println("Switching to RED (5s)");
        digitalWrite(LED_GREEN, LOW);
        break;
    }
  }
}