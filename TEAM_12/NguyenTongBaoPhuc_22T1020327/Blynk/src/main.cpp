#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#define BLYNK_TEMPLATE_ID "TMPL665DqNu9i"
#define BLYNK_TEMPLATE_NAME "Blynk IoT"
#define BLYNK_AUTH_TOKEN "CE_M5TCAlHMyfMbO7Vyrq0QAXoa3wyi0"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST"; // Tên mạng WiFi
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

unsigned long startTime = 0;
unsigned long elapsedTime = 0;
bool isRunning = false;
bool displayEnabled = false;

// ===== BUTTON APP (V4) =====
BLYNK_WRITE(V4)
{
  int state = param.asInt();

  digitalWrite(LED_PIN, state);
  displayEnabled = state;

  if (!displayEnabled)
  {
    display.clear();   // tắt bảng khi switch OFF
  }
}

// ===== UPTIME =====
void sendUptime()
{
  unsigned long elapsedTime = millis() / 1000;

  // Gửi lên Blynk (luôn luôn gửi)
  Blynk.virtualWrite(V0, elapsedTime);

  // Nếu switch bật thì mới hiển thị bảng
  if (displayEnabled)
  {
    int minutes = elapsedTime / 60;
    int seconds = elapsedTime % 60;

    int displayValue = minutes * 100 + seconds;

    display.showNumberDecEx(displayValue, true);
  }
}
// ===== SENSOR =====
void sendSensor()
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h))
  {
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  display.setBrightness(7);
  display.clear();
  timer.setInterval(1000L, sendUptime);
  timer.setInterval(2000L, sendSensor);
}

void loop()
{

  Blynk.run();
  timer.run();
}