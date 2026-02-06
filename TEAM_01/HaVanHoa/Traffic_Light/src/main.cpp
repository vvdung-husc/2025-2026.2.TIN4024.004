#include <Arduino.h>
#include <TM1637Display.h>

#define PIN_LED_RED    25
#define PIN_LED_YELLOW 33
#define PIN_LED_GREEN  32
#define PIN_LED_BLUE   21 
#define PIN_BUTTON     23
#define PIN_LDR        13   // Chân cảm biến ánh sáng theo sơ đồ của bạn

#define CLK 18
#define DIO 19

// Ngưỡng ánh sáng: Bạn có thể điều chỉnh số này tùy theo độ sáng trong Wokwi
#define LDR_THRESHOLD  1500 

TM1637Display display(CLK, DIO);
enum TrafficState {
  STATE_GREEN,
  STATE_YELLOW,
  STATE_RED
};

TrafficState currentState = STATE_GREEN;
int counter = 6;           
bool ledStatus = false;    
unsigned long ulTimer = 0; 

bool isDisplayEnabled = true;
int lastButtonState = HIGH;

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
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  display.setBrightness(0x0f);

  Serial.printf("LED [GREEN ] ON => 7 Seconds\r\n");
  digitalWrite(PIN_LED_BLUE, HIGH);
  
  if(isDisplayEnabled) display.showNumberDec(counter);
}

void loop() {
  // --- Đọc cảm biến ánh sáng ---
  int ldrValue = analogRead(PIN_LDR);
  bool isNight = (ldrValue < LDR_THRESHOLD);

  // Giữ nguyên logic nút bấm của bạn
  int currentButtonState = digitalRead(PIN_BUTTON);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    isDisplayEnabled = !isDisplayEnabled; 
    digitalWrite(PIN_LED_BLUE, isDisplayEnabled ? HIGH : LOW);
    if (!isDisplayEnabled) {
      display.clear();
    } else {
      display.showNumberDec(counter); 
    }
    delay(50); 
  }
  lastButtonState = currentButtonState;

  if (IsReady(ulTimer, 500)) { 
    ledStatus = !ledStatus; 

    if (isNight) {
      // --- CHẾ ĐỘ BAN ĐÊM: CHỈ NHẤP NHÁY VÀNG ---
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_YELLOW, ledStatus ? HIGH : LOW);
      display.clear(); // Tắt số khi ban đêm
    } 
    else {
      // --- CHẾ ĐỘ BAN NGÀY: GIỮ NGUYÊN CODE CŨ CỦA BẠN ---
      switch (currentState) {
        case STATE_GREEN:
          digitalWrite(PIN_LED_GREEN, ledStatus ? HIGH : LOW);
          break;
        case STATE_YELLOW:
          digitalWrite(PIN_LED_YELLOW, ledStatus ? HIGH : LOW);
          break;
        case STATE_RED:
          digitalWrite(PIN_LED_RED, ledStatus ? HIGH : LOW);
          break;
      }

      if (!ledStatus) { 
        const char* colorName = (currentState == STATE_GREEN) ? "GREEN" : (currentState == STATE_YELLOW) ? "YELLOW" : "RED ";
        Serial.printf("[%s] => Seconds: %d\r\n", colorName, counter);

        if (isDisplayEnabled) {
          display.showNumberDec(counter);
        }

        counter--; 

        if (counter < 0) {
digitalWrite(PIN_LED_GREEN, LOW);
          digitalWrite(PIN_LED_YELLOW, LOW);
          digitalWrite(PIN_LED_RED, LOW);

          switch (currentState) {
            case STATE_GREEN:
              currentState = STATE_YELLOW;
              counter = 2; 
              Serial.printf("LED [YELLOW] ON => 3 Seconds\r\n");
              break;
            case STATE_YELLOW:
              currentState = STATE_RED;
              counter = 4; 
              Serial.printf("LED [RED   ] ON => 5 Seconds\r\n");
              break;
            case STATE_RED:
              currentState = STATE_GREEN;
              counter = 6; 
              Serial.printf("LED [GREEN ] ON => 7 Seconds\r\n");
              break;
          }

          if (isDisplayEnabled) {
             display.showNumberDec(counter);
          }
        }
      }
    }
  }
}