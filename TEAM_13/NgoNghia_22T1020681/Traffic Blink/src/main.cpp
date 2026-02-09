#include <Arduino.h>

#define RED_LED 25
#define YELLOW_LED 33
#define GREEN_LED 32

int state = 1;
int lastState = 0;

unsigned long previousMillis = 0;
unsigned long blinkMillis = 0;
bool ledBlink = false;

const unsigned long redTime = 5000;
const unsigned long greenTime = 7000;
const unsigned long yellowTime = 3000;

void setup()
{
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  Serial.begin(115200);
}

void loop()
{
  unsigned long now = millis();

  if (state != lastState)
  {
    lastState = state;
    blinkMillis = now;
    ledBlink = false;

    switch (state)
    {
      case 1:
        Serial.printf("LED[RED] ON => %lu Seconds", redTime / 1000);
        Serial.println();
        break;
      case 2:
        Serial.printf("LED[GREEN] ON => %lu Seconds", greenTime / 1000);
        Serial.println();
        break;
      case 3:
        Serial.printf("LED[YELLOW] ON => %lu Seconds", yellowTime / 1000);
        Serial.println();
        break;
    }
  }

  switch (state)
  {
    case 1: // RED BLINK
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(GREEN_LED, LOW);

      if (now - blinkMillis >= 500)
      {
        blinkMillis = now;
        ledBlink = !ledBlink;
        digitalWrite(RED_LED, ledBlink);
      }

      if (now - previousMillis >= redTime)
      {
        ledBlink = false;
        previousMillis = now;
        digitalWrite(RED_LED, LOW);
        state = 2;
      }
      break;

    case 2: // GREEN BLINK
      digitalWrite(RED_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);

      if (now - blinkMillis >= 500)
      {
        blinkMillis = now;
        ledBlink = !ledBlink;
        digitalWrite(GREEN_LED, ledBlink);
      }

      if (now - previousMillis >= greenTime)
      {
        ledBlink = false;
        previousMillis = now;
        digitalWrite(GREEN_LED, LOW);
        state = 3;
      }
      break;

    case 3: // YELLOW BLINK
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);

      if (now - blinkMillis >= 500)
      {
        blinkMillis = now;
        ledBlink = !ledBlink;
        digitalWrite(YELLOW_LED, ledBlink);
      }

      if (now - previousMillis >= yellowTime)
      {
        ledBlink = false;
        previousMillis = now;
        digitalWrite(YELLOW_LED, LOW);
        state = 1;
      }
      break;
  }
}