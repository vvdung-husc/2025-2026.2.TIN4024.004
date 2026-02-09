#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

// Non-blocking timer
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.println("WELCOME IOT");
}

void loop() {
  static unsigned long ulTimer = 0;
  static uint8_t state = 0;  // 0=RED, 1=YELLOW, 2=GREEN

  switch (state) {
    case 0: // RED
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);

      if (IsReady(ulTimer, 5000)) {
        Serial.println("LED [RED] ON => 5 Seconds");
        state = 1;
      }
      break;

    case 1: // YELLOW
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, HIGH);
      digitalWrite(LED_GREEN, LOW);

      if (IsReady(ulTimer, 3000)) {
        Serial.println("LED [YELLOW] ON => 3 Seconds");
        state = 2;
      }
      break;

    case 2: // GREEN
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, HIGH);

      if (IsReady(ulTimer, 7000)) {
        Serial.println("LED [GREEN] ON => 7 Seconds");
        state = 0;
      }
      break;
  }
}


//BLOCKING
// void loop() {
//   // put your main code here, to run repeatedly:
//   // static int i = 0;
//   // printf("Loop running ...%d\n",++i);
//   // delay(1000);

//   digitalWrite(PIN_LED_RED, HIGH); // Turn LED ON
//   delay(500); // Wait for 500ms
//   digitalWrite(PIN_LED_RED, LOW); // Turn LED OFF
//   delay(500); // Wait for 500ms  
// }

