#include <Arduino.h>

#define RED_LED     25
#define YELLOW_LED  33
#define GREEN_LED   32

enum State {
  RED,
  YELLOW,
  GREEN
};

State currentState = RED;

unsigned long stateTimer = 0;   // đổi màu
unsigned long blinkTimer = 0;   // nhấp nháy
bool ledState = false;

// Thời gian mỗi màu (ms)
const unsigned long RED_TIME    = 5000;
const unsigned long YELLOW_TIME = 3000;
const unsigned long GREEN_TIME  = 7000;

// Chu kỳ blink (ms)
const unsigned long BLINK_TIME  = 500;

void turnOffAll() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  stateTimer = millis();
  blinkTimer = millis();
}

void loop() {
  unsigned long now = millis();

  // ===== BLINK =====
  if (now - blinkTimer >= BLINK_TIME) {
    blinkTimer = now;
    ledState = !ledState;

    turnOffAll();

    if (ledState) {
      if (currentState == RED)     digitalWrite(RED_LED, HIGH);
      if (currentState == YELLOW)  digitalWrite(YELLOW_LED, HIGH);
      if (currentState == GREEN)   digitalWrite(GREEN_LED, HIGH);
    }
  }

  // ===== STATE CHANGE =====
  switch (currentState) {
    case RED:
      if (now - stateTimer >= RED_TIME) {
        currentState = YELLOW;
        stateTimer = now;
        Serial.println("LED [YELLOW] => 3 Seconds (BLINK)");
      }
      break;

    case YELLOW:
      if (now - stateTimer >= YELLOW_TIME) {
        currentState = GREEN;
        stateTimer = now;
        Serial.println("LED [GREEN ] => 7 Seconds (BLINK)");
      }
      break;

    case GREEN:
      if (now - stateTimer >= GREEN_TIME) {
        currentState = RED;
        stateTimer = now;
        Serial.println("LED [RED   ] => 5 Seconds (BLINK)");
      }
      break;
  }
}
