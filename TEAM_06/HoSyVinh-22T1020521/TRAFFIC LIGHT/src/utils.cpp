#include "utils.h"

#define LED_RED     26
#define LED_YELLOW  33
#define LED_GREEN   32
#define LED_BLUE    21
#define BUTTON_PIN  23

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

bool extraMode = false;
bool lastButtonState = HIGH;

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

void turnOffAllTrafficLED()
{
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
}

void showCountdown(const char* color, int seconds)
{
    Serial.print("LED [");
    Serial.print(color);
    Serial.print("] ON => ");
    Serial.print(seconds);
    Serial.println(" seconds");

    for (int i = seconds; i >= 1; i--)
    {
        handleButton(); // <<< QUAN TRONG NHAT

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