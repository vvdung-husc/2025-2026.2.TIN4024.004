#define BLYNK_TEMPLATE_ID "TMPL69TudFJH4"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "vhbMTd7zkt2xmFD4h2V0sYwXbij4jJsG"  

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";   
char pass[] = "";              

#define PIN_LED_RED 23
#define PIN_LED_GREEN 22
#define PIN_LED_YELLOW 21
#define PIN_LED_BLUE 26
#define PIN_BUTTON 27
#define PIN_LDR 35

#define CLK 32
#define DIO 33

TM1637Display display(CLK, DIO);
BlynkTimer timer;

int counter = 0;

int redTime = 15;
int yellowTime = 5;
int greenTime = 15;

int state = 0;
int sec = 0;

void sendData()
{
  Blynk.virtualWrite(V3, counter);
   Blynk.virtualWrite(V1, random(20, 35));   
  Blynk.virtualWrite(V2, random(40, 80));   
  Blynk.virtualWrite(V3, counter);

}

void trafficRun()
{
  int ldr = analogRead(PIN_LDR);

  
  if (ldr < 500)
  {
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    display.clear();
    return;
  }

  sec--;

  if (sec <= 0)
  {
    state++;

    if (state > 2)
      state = 0;

    if (state == 0)
    {
      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_YELLOW, LOW);
      sec = redTime;
    }

    if (state == 1)
    {
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, HIGH);
      digitalWrite(PIN_LED_YELLOW, LOW);
      sec = greenTime;
    }

    if (state == 2)
    {
      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, LOW);
      digitalWrite(PIN_LED_YELLOW, HIGH);
      sec = yellowTime;
    }
  }

  display.showNumberDec(sec);
  counter++;
}

void handleButton()
{
  bool btn = digitalRead(PIN_BUTTON);

  if (btn == LOW)
    digitalWrite(PIN_LED_BLUE, HIGH);
  else
    digitalWrite(PIN_LED_BLUE, LOW);
}

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LDR, INPUT);

  display.setBrightness(7);
  display.clear();

  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);

  sec = redTime;

  timer.setInterval(1000L, trafficRun);
  timer.setInterval(2000L, sendData);
}

void loop()
{
  Blynk.run();
  timer.run();
  handleButton();
}