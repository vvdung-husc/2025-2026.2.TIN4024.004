#define BLYNK_TEMPLATE_ID "TMPL6Jq8n-SMC"
#define BLYNK_TEMPLATE_NAME "Hà Văn Hòa"
#define BLYNK_AUTH_TOKEN "Z6AaCUWeBtYACR9wlENMHDOHselKTJk0"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define LED_PIN 21
#define BUTTON_PIN 23
#define DHTPIN 16
#define DHTTYPE DHT22

#define CLK 18
#define DIO 19

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

BlynkTimer timer;

int timeCounter = 0;
bool running = false;

void sendDHT()
{
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, hum);
}

void updateTime()
{
  if (running)
  {
    timeCounter++; // đếm tăng
    display.showNumberDec(timeCounter);

    Blynk.virtualWrite(V3, timeCounter);
  }
}

BLYNK_WRITE(V0)
{
  int value = param.asInt();

  digitalWrite(LED_PIN, value);
  running = value;
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();

  display.setBrightness(7);
  display.showNumberDec(0);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, updateTime);
  timer.setInterval(2000L, sendDHT);
  
}

void loop()
{
  Blynk.run();
  timer.run();

  if (digitalRead(BUTTON_PIN) == LOW)
  {
    running = !running;

    digitalWrite(LED_PIN, running);

    Blynk.virtualWrite(V0, running);

    delay(300);
  }
}