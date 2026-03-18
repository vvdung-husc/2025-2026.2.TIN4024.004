#include <Arduino.h>
#include <TM1637Display.h>

#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32
#define PIN_LED_BLUE    21
#define PIN_BUTTON_DISPLAY 23

#define CLK 15
#define DIO 2

TM1637Display display(CLK, DIO);

enum TrafficState {GREEN, YELLOW, RED};
TrafficState currentState = GREEN;

unsigned long previousMillis = 0;
int countdown = 7; // bắt đầu đèn xanh 7s
bool displayEnabled = false;

void setTrafficLED(TrafficState state)
{
  digitalWrite(PIN_LED_GREEN, state == GREEN);
  digitalWrite(PIN_LED_YELLOW, state == YELLOW);
  digitalWrite(PIN_LED_RED, state == RED);
}

void nextState()
{
  switch (currentState)
  {
    case GREEN:
      currentState = YELLOW;
      countdown = 3;
      break;
    case YELLOW:
      currentState = RED;
      countdown = 5;
      break;
    case RED:
      currentState = GREEN;
      countdown = 7;
      break;
  }
  setTrafficLED(currentState);
}

void handleButton()
{
  static int lastState = LOW;
  int reading = digitalRead(PIN_BUTTON_DISPLAY);

  if (reading != lastState)
  {
    delay(5); // debounce nhẹ
    if (reading == HIGH)
    {
      displayEnabled = !displayEnabled;
      digitalWrite(PIN_LED_BLUE, displayEnabled);
      if (!displayEnabled) display.clear();
    }
    lastState = reading;
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUTTON_DISPLAY, INPUT);

  display.setBrightness(7);
  setTrafficLED(currentState);
}

void loop()
{
  handleButton();

  unsigned long currentMillis = millis();

  // mỗi 1 giây giảm countdown
  if (currentMillis - previousMillis >= 1000)
  {
    previousMillis = currentMillis;
    countdown--;

    if (displayEnabled)
      display.showNumberDec(countdown, true);

    Serial.printf("State: %d | Time left: %d\n", currentState, countdown);

    if (countdown <= 0)
      nextState();
  }
}
