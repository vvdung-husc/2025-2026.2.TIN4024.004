#include "utils.h"
#include <math.h>

// Pin define
#define LED_RED     26
#define LED_YELLOW  33
#define LED_GREEN   32
#define LED_BLUE    21
#define BUTTON_PIN  23
#define LDR_PIN     13

#define CLK 18
#define DIO 19

// LDR constant (Wokwi)
#define RL10 20
#define GAMMA 1.2

TM1637Display display(CLK, DIO);

bool extraMode = false;
bool lastButtonState = HIGH;

// Read lux from LDR
float readLux()
{
    int analogValue = analogRead(LDR_PIN);
    float voltage = analogValue / 4095.0 * 5.0;
    float resistance = 2000 * voltage / (5.0 - voltage);
    float lux = pow(RL10 * 1000 * pow(10, GAMMA) / resistance, (1.0 / GAMMA));
    return lux;
}

// Handle push button
void handleButton()
{
    bool currentState = digitalRead(BUTTON_PIN);

    if (lastButtonState == HIGH && currentState == LOW)
    {
        extraMode = !extraMode;

        if (extraMode)
        {
            digitalWrite(LED_BLUE, HIGH);
            display.setBrightness(7);
        }
        else
        {
            digitalWrite(LED_BLUE, LOW);
            display.clear();
        }

        delay(200); // debounce
    }

    lastButtonState = currentState;
}

// Turn off traffic LEDs
void turnOffAllTrafficLED()
{
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
}

// Countdown with Serial + TM1637
void showCountdown(const char* color, int seconds)
{
    Serial.print("LED [");
    Serial.print(color);
    Serial.print("] ON => ");
    Serial.print(seconds);
    Serial.println(" seconds");

    for (int i = seconds; i >= 1; i--)
    {
        handleButton();

        float lux = readLux();

        if (lux < 50)
        {
            turnOffAllTrafficLED();
            digitalWrite(LED_YELLOW, HIGH);
            Serial.println("LUX < 50 -> FORCE YELLOW");
        }

        Serial.print("[");
        Serial.print(color);
        Serial.print("] => seconds: ");
        Serial.println(i);

        if (extraMode)
        {
            display.showNumberDec(i, true);
        }

        delay(1000);
    }
}
