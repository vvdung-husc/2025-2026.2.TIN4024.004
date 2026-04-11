#include <Arduino.h>

// GPIO pins (match your diagram)
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

// Non-blocking timer function
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("WELCOME IOT - 3 LED NON BLOCKING");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  // Make sure all LEDs are OFF at start
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
}

void loop() {
  static unsigned long ulTimer = 0;
  static uint8_t state = 0;  // 0=RED, 1=YELLOW, 2=GREEN

  if (IsReady(ulTimer, 1000)) {
    // Turn all LEDs OFF first
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);

    switch (state) {
      case 0:
        digitalWrite(PIN_LED_RED, HIGH);
        Serial.println("RED ON");
        break;

      case 1:
        digitalWrite(PIN_LED_YELLOW, HIGH);
        Serial.println("YELLOW ON");
        break;

      case 2:
        digitalWrite(PIN_LED_GREEN, HIGH);
        Serial.println("GREEN ON");
        break;
    }

    state++;
    if (state > 2) state = 0; // loop back
  }
}
