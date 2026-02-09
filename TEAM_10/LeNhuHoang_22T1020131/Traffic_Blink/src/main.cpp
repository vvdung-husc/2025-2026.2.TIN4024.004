#include <Arduino.h>

#define PIN_LED_RED     26
#define PIN_LED_YELLOW  25
#define PIN_LED_GREEN   33

// Non-blocking timer
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

enum TrafficState {
  GREEN,
  YELLOW,
  RED
};

void setup() {
  printf("WELCOME TRAFFIC IOT\n");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  // Start with GREEN
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, LOW);
}

void loop() {
  static unsigned long ulTimer = 0;
  static TrafficState state = GREEN;

  switch (state) {

    case GREEN:
      if (IsReady(ulTimer, 7000)) {
        digitalWrite(PIN_LED_GREEN, LOW);
        digitalWrite(PIN_LED_YELLOW, HIGH);
        state = YELLOW;
        printf("STATE: GREEN -> YELLOW\n");
      }
      break;

    case YELLOW:
      if (IsReady(ulTimer, 3000)) {
        digitalWrite(PIN_LED_YELLOW, LOW);
        digitalWrite(PIN_LED_RED, HIGH);
        state = RED;
        printf("STATE: YELLOW -> RED\n");
      }
      break;

    case RED:
      if (IsReady(ulTimer, 5000)) {
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_LED_GREEN, HIGH);
        state = GREEN;
        printf("STATE: RED -> GREEN\n");
      }
      break;
  }
}
