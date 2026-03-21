#include <Arduino.h>


#define LED_GREEN  32
#define LED_YELLOW 33
#define LED_RED    25


enum TrafficState {
  RED,
  YELLOW,
  GREEN
};

TrafficState currentState = RED;


unsigned long redTime    = 5000;
unsigned long yellowTime = 3000;
unsigned long greenTime  = 7000;

unsigned long previousMillis = 0;

void setLights(bool red, bool yellow, bool green) {
  digitalWrite(LED_RED, red);
  digitalWrite(LED_YELLOW, yellow);
  digitalWrite(LED_GREEN, green);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  
  setLights(true, false, false);
  Serial.println("LED [RED   ] ON => 5 Seconds");
}

void loop() {
  unsigned long currentMillis = millis();

  switch (currentState) {

    case RED:
      if (currentMillis - previousMillis >= redTime) {
        previousMillis = currentMillis;
        currentState = YELLOW;

        setLights(false, true, false);
        Serial.println("LED [YELLOW] ON => 3 Seconds");
      }
      break;

    case YELLOW:
      if (currentMillis - previousMillis >= yellowTime) {
        previousMillis = currentMillis;
        currentState = GREEN;

        setLights(false, false, true);
        Serial.println("LED [GREEN ] ON => 7 Seconds");
      }
      break;

    case GREEN:
      if (currentMillis - previousMillis >= greenTime) {
        previousMillis = currentMillis;
        currentState = RED;

        setLights(true, false, false);
        Serial.println("LED [RED   ] ON => 5 Seconds");
      }
      break;
  }
}
