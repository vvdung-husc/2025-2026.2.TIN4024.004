#include <Arduino.h>
#include <TM1637Display.h>
// Định nghĩa chân (theo sơ đồ Wokwi ban đầu)
#define PIN_LED_RED    25
#define PIN_LED_YELLOW 33
#define PIN_LED_GREEN  32

// Định nghĩa trạng thái
enum TrafficState {
  STATE_GREEN,
  STATE_YELLOW,
  STATE_RED
};

// Hàm Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
}

void loop() {
  static unsigned long ulTimer = 0;
  static bool ledStatus = false;
  
  static TrafficState currentState = STATE_GREEN;
  static int counter = 7; 

  // Chu kỳ 500ms
  if (IsReady(ulTimer, 500)) { 
    ledStatus = !ledStatus; // Đảo trạng thái

    switch (currentState) {
      
      // --- XANH (7s) ---
      case STATE_GREEN:
        digitalWrite(PIN_LED_GREEN, ledStatus ? HIGH : LOW);
        
        // Chỉ đếm khi đèn chuyển từ Sáng -> Tắt (trọn 1 giây)
        if (!ledStatus) {
           counter--;
           if (counter <= 0) {
             // Đợi hết 7 giây mới in
             printf("LED [GREEN ] ON => 7 Seconds\n");
             
             // Chuyển trạng thái
             currentState = STATE_YELLOW;
             counter = 3; 
             digitalWrite(PIN_LED_GREEN, LOW);
           }
        }
        break;

      // --- VÀNG (3s) ---
      case STATE_YELLOW:
        digitalWrite(PIN_LED_YELLOW, ledStatus ? HIGH : LOW);
        
        if (!ledStatus) {
           counter--;
           if (counter <= 0) {
             // Đợi hết 3 giây mới in
             printf("LED [YELLOW] ON => 3 Seconds\n");
             
             currentState = STATE_RED;
             counter = 5; 
             digitalWrite(PIN_LED_YELLOW, LOW);
           }
        }
        break;

      // --- ĐỎ (5s) ---
      case STATE_RED:
        digitalWrite(PIN_LED_RED, ledStatus ? HIGH : LOW);
        
        if (!ledStatus) {
           counter--;
           if (counter <= 0) {
             // Đợi hết 5 giây mới in
             printf("LED [RED   ] ON => 5 Seconds\n");
             
             currentState = STATE_GREEN;
             counter = 7; 
             digitalWrite(PIN_LED_RED, LOW);
           }
        }
        break;
    }
  }
}