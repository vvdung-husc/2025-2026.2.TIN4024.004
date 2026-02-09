#include <Arduino.h>

#define PIN_LED_RED     23
#define PIN_LED_YELLOW  22
#define PIN_LED_GREEN   21

enum TrafficState {
  STATE_GREEN,
  STATE_YELLOW,
  STATE_RED
};

unsigned long stateTimer = 0;
TrafficState currentState = STATE_GREEN;

void setLights(bool red, bool yellow, bool green) {
  digitalWrite(PIN_LED_RED, red);
  digitalWrite(PIN_LED_YELLOW, yellow);
  digitalWrite(PIN_LED_GREEN, green);
}

void setup() {
  Serial.begin(115200);
  printf("WELCOME IOT\n");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  // Bắt đầu với đèn xanh
  setLights(LOW, LOW, HIGH);
  stateTimer = millis();
}

void loop() {
  unsigned long now = millis();

  switch (currentState) {

    case STATE_GREEN:
      if (now - stateTimer >= 7000) {
        printf("LED [GREEN ] ON => 7 Seconds\n");
        setLights(LOW, HIGH, LOW);
        currentState = STATE_YELLOW;
        stateTimer = now;
      }
      break;

    case STATE_YELLOW:
      if (now - stateTimer >= 3000) {
        printf("LED [YELLOW] ON => 3 Seconds\n");
        setLights(HIGH, LOW, LOW);
        currentState = STATE_RED;
        stateTimer = now;
      }
      break;

    case STATE_RED:
      if (now - stateTimer >= 5000) {
        printf("LED [RED   ] ON => 5 Seconds\n");
        setLights(LOW, LOW, HIGH);
        currentState = STATE_GREEN;
        stateTimer = now;
      }
      break;
  }
}