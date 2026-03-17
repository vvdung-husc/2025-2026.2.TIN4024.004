#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>


#define BLYNK_TEMPLATE_ID "TMPL69ZGu9hFg"
#define BLYNK_TEMPLATE_NAME "Esp32blynk"
#define BLYNK_AUTH_TOKEN "JyuKUoYYd2Q27eZapfIdy9x4-zvyLMb9"


// WiFi
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

// ===== DHT22 =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_PIN 21

// ===== Button =====
#define BTN_PIN 23

// ===== TM1637 =====
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// ===== Timer =====
BlynkTimer timer;

// ===== Variables =====
unsigned long startTime;
int runTime = 0;
bool ledState = false;

// ===== Blynk Button =====
BLYNK_WRITE(V0)
{
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

// ===== Read DHT =====
void sendDHT()
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
  }
}

// ===== Runtime counter =====
void updateRuntime()
{
  runTime = (millis() - startTime) / 1000;

  display.showNumberDec(runTime, true);

  Blynk.virtualWrite(V3, runTime);
}

// ===== Button control =====
void checkButton()
{
  if (digitalRead(BTN_PIN) == LOW)
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Blynk.virtualWrite(V0, ledState);
    delay(300);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  dht.begin();

  display.setBrightness(7);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  startTime = millis();

  timer.setInterval(2000L, sendDHT);
  timer.setInterval(1000L, updateRuntime);
}

void loop()
{
  Blynk.run();
  timer.run();
  checkButton();
}
