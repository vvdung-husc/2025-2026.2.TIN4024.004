#include <Arduino.h>

#define PIN_LED_RED 23

//Non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

void setup() {
  // put your setup code here, to run once:
  printf("WELCOME IOT\n");
  pinMode(PIN_LED_RED, OUTPUT);
}

//Non-Blocking
void loop() {
  static unsigned long ulTimer = 0;
  static bool lesStatus = false;
  if (IsReady(ulTimer, 500)){
    lesStatus = !lesStatus;
    printf("LES IS [%s]\n",lesStatus ? "ON" : "OFF");
    digitalWrite(PIN_LED_RED, lesStatus ? HIGH : LOW);
  }
}