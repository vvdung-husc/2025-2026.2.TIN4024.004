#include <Arduino.h>

#define PIN_LED_RED     23
#define PIN_LED_YELLOW  22
#define PIN_LED_GREEN   21

#define LED_ON   HIGH
#define LED_OFF  LOW

enum TrafficState {
  STATE_GREEN,
  STATE_YELLOW,
  STATE_RED
};

unsigned long stateTimer = 0;
unsigned long blinkTimer = 0;

const unsigned long BLINK_INTERVAL = 500;
bool blinkState = false;

TrafficState currentState = STATE_GREEN;

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  digitalWrite(PIN_LED_RED, LED_OFF);
  digitalWrite(PIN_LED_YELLOW, LED_OFF);
  digitalWrite(PIN_LED_GREEN, LED_OFF);

  stateTimer = millis();
  blinkTimer = millis();
}

void loop() {
  unsigned long now = millis();

  //Xử lý blink LED
  if (now - blinkTimer >= BLINK_INTERVAL) {
    blinkState = !blinkState;
    blinkTimer = now;

    digitalWrite(PIN_LED_RED, LED_OFF);
    digitalWrite(PIN_LED_YELLOW, LED_OFF);
    digitalWrite(PIN_LED_GREEN, LED_OFF);

    switch (currentState) {
      case STATE_GREEN:
        digitalWrite(PIN_LED_GREEN, blinkState ? LED_ON : LED_OFF);
        break;

      case STATE_YELLOW:
        digitalWrite(PIN_LED_YELLOW, blinkState ? LED_ON : LED_OFF);
        break;

      case STATE_RED:
        digitalWrite(PIN_LED_RED, blinkState ? LED_ON : LED_OFF);
        break;
    }
  }

  switch (currentState) {

    case STATE_GREEN:
      if (now - stateTimer >= 7000) {
        printf("LED [GREEN ] ON => 7 Seconds\n");
        currentState = STATE_YELLOW;
        stateTimer = now;
      }
      break;

    case STATE_YELLOW:
      if (now - stateTimer >= 3000) {
        printf("LED [YELLOW] ON => 3 Seconds\n");
        currentState = STATE_RED;
        stateTimer = now;
      }
      break;

    case STATE_RED:
      if (now - stateTimer >= 5000) {
        printf("LED [RED ] ON => 5 Seconds\n");
        currentState = STATE_GREEN;
        stateTimer = now;
      }
      break;
  }
}
