#include "main.h"
#include "utils.h"

// Pin define
#define LED_RED     26
#define LED_YELLOW  33
#define LED_GREEN   32
#define LED_BLUE    21
#define BUTTON_PIN  23

void setupPins()
{
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    digitalWrite(LED_BLUE, LOW);
}

void setup()
{
    Serial.begin(115200);
    setupPins();

    Serial.println("=== ESP32 TRAFFIC LIGHT SYSTEM ===");
}

void loopTrafficLight()
{
    // Green
    turnOffAllTrafficLED();
    digitalWrite(LED_GREEN, HIGH);
    showCountdown("GREEN", 6);

    // Red
    turnOffAllTrafficLED();
    digitalWrite(LED_RED, HIGH);
    showCountdown("RED", 4);

    // Yellow
    turnOffAllTrafficLED();
    digitalWrite(LED_YELLOW, HIGH);
    showCountdown("YELLOW", 2);
}

void loop()
{
    handleButton();
    loopTrafficLight();
}
