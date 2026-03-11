
#define BLYNK_TEMPLATE_ID "TMPL6T3QAEvuv"
#define BLYNK_TEMPLATE_NAME "BLYNK LED"
#define BLYNK_AUTH_TOKEN "OqZ76U4Z4jhPlQfOGtfx-E0gl5YpPejT"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>


#define LED_PIN 21
#define BTN_PIN 23
#define DHT_PIN 16
#define DHTTYPE DHT22

#define CLK 18
#define DIO 19


char ssid[] = "Wokwi-GUEST";
char pass[] = "";


DHT dht(DHT_PIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;


bool systemOn = false;          
unsigned long secondsCount = 0; 

int buttonState;             
int lastButtonState = HIGH;   
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;


void updateSystemState() {
  if (systemOn) {
    digitalWrite(LED_PIN, HIGH);         
    display.showNumberDec(secondsCount); 
  } else {
    digitalWrite(LED_PIN, LOW);          
    display.clear();                     
  }
  Blynk.virtualWrite(V0, systemOn); 
}


BLYNK_WRITE(V0) {
  systemOn = param.asInt();
  updateSystemState();
}


void countTime() {
  secondsCount++; 
  
  if (systemOn) {
    display.showNumberDec(secondsCount);
  }
  String timeString = String(secondsCount) + " Giây";
  Blynk.virtualWrite(V1, timeString);
}


void sendSensor() {

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("Nhiệt độ: "); Serial.print(t);
  Serial.print(" *C  ---  Độ ẩm: "); Serial.print(h);
  Serial.println(" %");
  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V2, t); 
    Blynk.virtualWrite(V3, h); 
  } else {
    Serial.println("CẢNH BÁO: Không đọc được dữ liệu từ DHT22!");
  }
}


void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  display.setBrightness(0x0f);
  display.clear();
  digitalWrite(LED_PIN, LOW);
  
  dht.begin();
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  timer.setInterval(1000L, countTime);   
  timer.setInterval(2000L, sendSensor);   
}

void loop() {
  Blynk.run();
  timer.run(); 
  int reading = digitalRead(BTN_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == LOW) {
        systemOn = !systemOn; 
        updateSystemState();  
      }
    }
  }
  lastButtonState = reading; 
}