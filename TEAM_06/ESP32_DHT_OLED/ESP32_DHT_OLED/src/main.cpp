/*
THÔNG TIN NHÓM 6
1. Nguyễn Hữu Trình
2. Ngô Thời Quang
3. Hồ Sỹ Vinh
4. Nguyễn Đinh Vĩnh Phú
5. Nguyễn Đăng Bảo Toàn
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>


#define PIN_DHT     16  
#define DHT_TYPE    DHT22

#define PIN_GREEN   15  
#define PIN_YELLOW  2  
#define PIN_RED     4 


#define PIN_OLED_SDA 13 
#define PIN_OLED_SCL 12

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


DHT dht(PIN_DHT, DHT_TYPE);


float t = 0; 
float h = 0;
unsigned long lastReadTime = 0; 
unsigned long lastBlinkTime = 0;
bool ledState = false; 

void setup() {
  Serial.begin(115200);
  
 
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_RED, OUTPUT);


  dht.begin();


  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED khởi động thất bại!"));
    for(;;);
  }
  

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println("WEATHER STATION");
  display.setCursor(30, 40);
  display.println("Loading...");
  display.display();
  delay(1000); 
}


String getStatusMessage(float temp) {
    if (temp < 13) return "TOO COLD";
    if (temp < 20) return "COLD";
    if (temp < 25) return "COOL";
    if (temp < 30) return "WARM";
    if (temp <= 35) return "HOT";
    return "TOO HOT";
}


void blinkLedByTemp(float temp) {
  if (isnan(temp)) {
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_YELLOW, LOW);
    digitalWrite(PIN_RED, LOW);
    return;
  }


  if (millis() - lastBlinkTime > 500) {
    lastBlinkTime = millis();
    ledState = !ledState;
    
  
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_YELLOW, LOW);
    digitalWrite(PIN_RED, LOW);

    if (ledState) {
        if (temp < 20) {
            digitalWrite(PIN_GREEN, HIGH);
        } 
        else if (temp >= 20 && temp < 30) {
            digitalWrite(PIN_YELLOW, HIGH);
        } 
        else { 
            digitalWrite(PIN_RED, HIGH);
        }
    }
  }
}

void loop() {

  if (millis() - lastReadTime > 2000) {
    lastReadTime = millis();
    
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();

    if (isnan(newT) || isnan(newH)) {
      Serial.println("Lỗi đọc DHT22!");
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Sensor Error!");
      display.display();
    } else {
      t = newT;
      h = newH;
      
 
      display.clearDisplay();
      

      display.drawRect(0, 0, 128, 64, WHITE);
      
  
      display.setTextSize(1);
      display.setCursor(5, 5);
      display.print("STT: ");
      display.println(getStatusMessage(t));
      
  
      display.drawLine(0, 18, 128, 18, WHITE);

 
      display.setTextSize(2);
      display.setCursor(5, 25);
      display.print(t, 1);
      display.setTextSize(1);
      display.print("o");
      display.setTextSize(2);
      display.print("C");


      display.setTextSize(1);
      display.setCursor(80, 25);
      display.print("Humid");
      display.setCursor(80, 40);
      display.print(h, 0);
      display.print("%");

      display.display();
    }
  }


  blinkLedByTemp(t);
}