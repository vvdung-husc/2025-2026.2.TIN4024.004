#define BLYNK_TEMPLATE_ID "TMPL6BErqh1q7"
#define BLYNK_TEMPLATE_NAME "BLYNK"
#define BLYNK_AUTH_TOKEN "C7ByGk0j8lEY3YbbaX_cveDKR90qE0gj"

#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== DATastream =====
#define V_TEMP     V0
#define V_HUM      V1
#define V_LED      V2
#define V_COUNTER  V3

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
BlynkTimer timer;

// ===== VARIABLES =====
int counter = 0;
bool ledState = false;

// ===== BLYNK CONTROL LED =====
BLYNK_WRITE(V_LED)
{
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
  if (ledState) {
    Serial.println("LED ON");
  } else {
    Serial.println("LED OFF");
  }
}

float tempGlobal = 0;
float humGlobal = 0;
// ===== READ SENSOR =====
void sendSensor()
{
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT read failed!");
    return;
  }

  // Serial.printf("Temp: %.2f | Hum: %.2f\r\n", temp, hum);
  tempGlobal = temp;
  humGlobal = hum;

  // Gửi lên Blynk
  Blynk.virtualWrite(V_TEMP, temp);
  Blynk.virtualWrite(V_HUM, hum);
}

// ===== TIMER COUNTER =====
void updateCounter()
{
  counter++;

  Blynk.virtualWrite(V_COUNTER, counter);

  display.showNumberDec(counter);

  // Serial.print("Time: ");
  // Serial.println(counter);
  Serial.printf("Time: %d | Temp: %.2f C | Hum: %.2f %%\r\n",
                counter, tempGlobal, humGlobal);
}

// ===== BUTTON CONTROL =====
void checkButton()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);

    Blynk.virtualWrite(V_LED, ledState);

    if (ledState) {
    Serial.println("LED ON");
  } else {
    Serial.println("LED OFF");
  }
    delay(200); // debounce
  }
}

// ===== SETUP =====
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);

  dht.begin();

  WiFi.begin(ssid, pass);
  Serial.print("[WiFi] Connecting");

  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}

  Serial.println("\r\n[WiFi] Connected!");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer
  timer.setInterval(900L, sendSensor);
  timer.setInterval(1000L, updateCounter);
}

// ===== LOOP =====
void loop()
{
  Blynk.run();
  timer.run();
  checkButton();
}