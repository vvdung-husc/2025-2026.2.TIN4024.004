#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

void blinkLed(int pin, int times, const char* name) {
  Serial.print("LED [");
  Serial.print(name);
  Serial.print("] ON => ");
  Serial.print(times);
  Serial.println(" Seconds");

  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

}

void loop() {
  blinkLed(LED_RED, 5, "RED");
  delay(1000);

  blinkLed(LED_YELLOW, 3, "YELLOW");
  delay(1000);

  blinkLed(LED_GREEN, 7, "GREEN");
  delay(3000);
}
