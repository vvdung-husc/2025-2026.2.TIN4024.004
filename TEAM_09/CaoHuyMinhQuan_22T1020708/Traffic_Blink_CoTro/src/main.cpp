#include <Arduino.h>

#define RED_LED     25
#define YELLOW_LED  33
#define GREEN_LED   32

#define LED_ON  HIGH
#define LED_OFF LOW

enum TrafficState {
  STATE_RED,
  STATE_YELLOW,
  STATE_GREEN
};

TrafficState currentState = STATE_RED;

unsigned long stateTimer = 0;
unsigned long blinkTimer = 0;

const unsigned long BLINK_INTERVAL = 500; 
bool blinkState = false;

void setup() {
  
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RED_LED, LED_OFF);
  digitalWrite(YELLOW_LED, LED_OFF);
  digitalWrite(GREEN_LED, LED_OFF);

  stateTimer = millis();
  blinkTimer = millis();
}

void loop() {
  unsigned long now = millis();

  //Xử lý blink
  if (now - blinkTimer >= BLINK_INTERVAL) {
    blinkTimer = now;
    blinkState = !blinkState;

    
    digitalWrite(RED_LED, LED_OFF);
    digitalWrite(YELLOW_LED, LED_OFF);
    digitalWrite(GREEN_LED, LED_OFF);

    
    switch (currentState) {
      case STATE_RED:
        digitalWrite(RED_LED, blinkState ? LED_ON : LED_OFF);
        break;

      case STATE_YELLOW:
        digitalWrite(YELLOW_LED, blinkState ? LED_ON : LED_OFF);
        break;

      case STATE_GREEN:
        digitalWrite(GREEN_LED, blinkState ? LED_ON : LED_OFF);
        break;
    }
  }

  
  switch (currentState) {

    case STATE_RED:
      if (now - stateTimer >= 5000) {
        printf("LED [RED ] ON => 5 Seconds\n");
        currentState = STATE_YELLOW;
        stateTimer = now;
      }
      break;

    case STATE_YELLOW:
      if (now - stateTimer >= 3000) {
        printf("LED [YELLOW] ON => 3 Seconds\n");
        currentState = STATE_GREEN;
        stateTimer = now;
      }
      break;

    case STATE_GREEN:
      if (now - stateTimer >= 7000) {
        printf("LED [GREEN ] ON => 7 Seconds\n");
        currentState = STATE_RED;
        stateTimer = now;
      }
      break;
  }
}
