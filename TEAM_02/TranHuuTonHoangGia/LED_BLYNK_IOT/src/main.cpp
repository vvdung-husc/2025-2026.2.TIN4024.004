#define BLYNK_TEMPLATE_ID "TMPL6kk58g6be"
#define BLYNK_TEMPLATE_NAME "LED BLYNK"
#define BLYNK_AUTH_TOKEN "lK7gMCcdNJgn1Qequ-8Dfh0wBHK68HLG"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

/* WIFI WOKWI */
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* PIN */
#define LED_PIN 21
#define BTN_PIN 23
#define DHTPIN 16
#define CLK 18
#define DIO 19
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

/******** LED ********/
BLYNK_WRITE(V1)
{
  int s = param.asInt();
  digitalWrite(LED_PIN, s);
}

/******** BUTTON ********/
void checkButton()
{
  static int last = HIGH;
  int now = digitalRead(BTN_PIN);

  if (last == HIGH && now == LOW)
  {
    int newState = !digitalRead(LED_PIN);
    digitalWrite(LED_PIN, newState);
    Blynk.virtualWrite(V1, newState);
  }

  last = now;
}

/******** SENSOR ********/
void sendSensor()
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("Temp: ");
  Serial.println(t);

  Serial.print("Hum: ");
  Serial.println(h);

  if (!isnan(t) && !isnan(h))
  {
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V0, h);
  }

  display.showNumberDec((int)t);
}

/******** SETUP ********/
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(7);

  Serial.println("Connecting WiFi...");

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);

  timer.setInterval(2000L, sendSensor);
  timer.setInterval(100L, checkButton);

  digitalWrite(LED_PIN, LOW);
}

/******** LOOP ********/
void loop()
{
  Blynk.run();
  timer.run();
}