#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

#define BLYNK_TEMPLATE_ID "TMPL6BErqh1q7"
#define BLYNK_TEMPLATE_NAME "BLYNK"
#define BLYNK_AUTH_TOKEN "C7ByGk0j8lEY3YbbaX_cveDKR90qE0gj"

#include <WiFi.h>
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
BLYNK_WRITE(V2)
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

  // gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V0, temp);   // Nhietdo
  Blynk.virtualWrite(V1, hum);    // Doam
}


// ===== TIMER COUNTER =====
void updateCounter()
{
  counter++;

  Blynk.virtualWrite(V3, counter); // Thoigianhoatdong

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

    Blynk.virtualWrite(V2, ledState);

    delay(200);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);

  dht.begin();

  Serial.begin(115200);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer chạy nền
  timer.setInterval(2000L, sendSensor);
  timer.setInterval(1000L, updateCounter);
}

void loop()
{
  Blynk.run();
  timer.run();
  checkButton();
}