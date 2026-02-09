#include <Arduino.h>
#define LED_RED    26
#define LED_YELLOW 33
#define LED_GREEN  32
// Define the GPIO pin for the LED 
// put function declarations here:
void setup(){
  // put your setup code here, to run once:
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);

    Serial.begin(115200);
    delay(1000);

    Serial.println("=== BAT DAU CHUONG TRINH LED ESP32 ===");
}

void loop(){
    // put your main code here, to run repeatedly:
    // Buoc 1: LED do //
    Serial.println("Buoc 1: BAT LED DO (GPIO 26)");
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    delay(1000);

    // Buoc 2: LED vang //
    Serial.println("Buoc 2: BAT LED VANG (GPIO 33)");
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, LOW);
    delay(1000);

    // Buoc 3: LED xanh //
    Serial.println("Buoc 3: BAT LED XANH (GPIO 32)");
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
    delay(1000);

    Serial.println("------------------------------");
}
// put function definitions here: 
int myFunction(int x, int y) { 
  return x + y; 
}