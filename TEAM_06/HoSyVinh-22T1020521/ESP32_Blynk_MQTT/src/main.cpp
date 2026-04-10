#define BLYNK_TEMPLATE_ID "TMPL6SlZKTuH0"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "D68IdfC3Jk7KhfuI4ZX4bfnrkzO0MGsx"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

const int LED_PIN = 15;
const int BTN_PIN = 4;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST"; 
char pass[] = ""; 

BlynkTimer timer;
int ledState = LOW;

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1);
}

BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

void checkButton() {
  static int lastBtnState = HIGH;
  int currentRead = digitalRead(BTN_PIN);

  if (currentRead == LOW && lastBtnState == HIGH) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Blynk.virtualWrite(V1, ledState); // Cập nhật trạng thái lên Blynk Web
    delay(50); 
  }
  lastBtnState = currentRead;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(100L, checkButton);
}

void loop() {
  Blynk.run();
  timer.run();
}