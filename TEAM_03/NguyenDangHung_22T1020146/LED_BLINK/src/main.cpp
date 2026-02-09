#include <Arduino.h>

int ledXanh = 25;  
int ledVang = 26;  
int ledDo   = 27;  

void setup() {
  pinMode(ledXanh, OUTPUT);
  pinMode(ledVang, OUTPUT);
  pinMode(ledDo, OUTPUT);
}

void blinkLed(int pin, int seconds) {
  for (int i = 0; i < seconds; i++) {
    digitalWrite(pin, HIGH);
    delay(1000);
    digitalWrite(pin, LOW);
    delay(1000);
  }
}

void loop() {
  blinkLed(ledDo, 5);

  blinkLed(ledVang, 3);

  blinkLed(ledXanh, 7);
}
