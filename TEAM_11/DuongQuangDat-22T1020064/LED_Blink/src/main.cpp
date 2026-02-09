#include <Arduino.h>
#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

// Thời gian blink (ms)
const unsigned long RED_INTERVAL    = 500;   // 0.5s
const unsigned long YELLOW_INTERVAL = 1000;  // 1s
const unsigned long GREEN_INTERVAL  = 1500;  // 1.5s

unsigned long prevRedTime = 0;
unsigned long prevYellowTime = 0;
unsigned long prevGreenTime = 0;

bool redState = false;
bool yellowState = false;
bool greenState = false;

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.begin(115200);
  Serial.println("ESP32 Blink without delay()");
}

void loop() {
  unsigned long currentTime = millis();

  // RED LED blink
  if (currentTime - prevRedTime >= RED_INTERVAL) {
    prevRedTime = currentTime;
    redState = !redState;
    digitalWrite(LED_RED, redState);
  }

  // YELLOW LED blink
  if (currentTime - prevYellowTime >= YELLOW_INTERVAL) {
    prevYellowTime = currentTime;
    yellowState = !yellowState;
    digitalWrite(LED_YELLOW, yellowState);
  }

  // GREEN LED blink
  if (currentTime - prevGreenTime >= GREEN_INTERVAL) {
    prevGreenTime = currentTime;
    greenState = !greenState;
    digitalWrite(LED_GREEN, greenState);
  }
}
