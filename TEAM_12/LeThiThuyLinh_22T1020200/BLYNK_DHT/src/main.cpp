#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

#define BLYNK_TEMPLATE_ID "TMPL6D42PJODH"
#define BLYNK_TEMPLATE_NAME "BLYNK IoT"
#define BLYNK_AUTH_TOKEN "0gWqshrFKhuelHkNxHtOq4fEVRp-EAKb"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN CONFIG =====
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define LED_PIN 21
#define BUTTON_PIN 23

#define DHTTYPE DHT22

// ===== OBJECT =====
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

// ===== TIMER =====
BlynkTimer timer;

// ===== VARIABLES =====
int counter = 0;
bool ledState = false;

// ===== BLYNK CONTROL LED =====
BLYNK_WRITE(V0)
{
  int value = param.asInt();
  ledState = value;
  digitalWrite(LED_PIN, ledState);
}

// ===== READ SENSOR =====
void sendSensor()
{
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT read failed");
    return;
  }

  Serial.print("Temp: ");
  Serial.println(temp);

  Serial.print("Humidity: ");
  Serial.println(hum);

  // gửi lên Blynk
  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, hum);
}

// ===== TIMER COUNTER =====
void updateCounter()
{
  counter++;

  // gửi lên Blynk
  Blynk.virtualWrite(V3, counter);

  // hiển thị lên TM1637
  display.showNumberDec(counter);

  Serial.print("Time: ");
  Serial.println(counter);
}

// ===== BUTTON CONTROL =====
void checkButton()
{
  if (digitalRead(BUTTON_PIN) == LOW)
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
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);

  dht.begin();

  // kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // timer
  timer.setInterval(2000L, sendSensor);   // đọc DHT mỗi 2s
  timer.setInterval(1000L, updateCounter); // đếm thời gian
}

void loop()
{
  Blynk.run();
  timer.run();

  checkButton();
}