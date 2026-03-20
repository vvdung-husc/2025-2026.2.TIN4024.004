#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ===== DHT22 =====
#define DHTPIN 16
#define DHTTYPE DHT22

// ===== LED =====
#define LED_RED 4
#define LED_YELLOW 2
#define LED_GREEN 15

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

// millis
unsigned long prevLED = 0;
unsigned long prevDHT = 0;

const long intervalLED = 300;   // nháy nhanh hơn
const long intervalDHT = 2000;

float temp;
float humi;

int activeLED = -1;
bool ledState = false;

String statusMsg;

void setup(){

Serial.begin(115200);

pinMode(LED_RED,OUTPUT);
pinMode(LED_YELLOW,OUTPUT);
pinMode(LED_GREEN,OUTPUT);

Wire.begin(13,12);

display.begin(SSD1306_SWITCHCAPVCC,0x3C);

display.clearDisplay();
display.display();

dht.begin();
}


void loop(){

unsigned long now = millis();


// ===== Đọc nhiệt độ =====

if(now-prevDHT>=intervalDHT){

prevDHT=now;

temp=dht.readTemperature();
humi=dht.readHumidity();

if(!isnan(temp)&&!isnan(humi)){

digitalWrite(LED_RED,LOW);
digitalWrite(LED_YELLOW,LOW);
digitalWrite(LED_GREEN,LOW);


// ===== NGƯỠNG =====

if(temp<13){

statusMsg="TOO COLD";
activeLED=LED_GREEN;

}

else if(temp>=13 && temp<20){

statusMsg="COLD";
activeLED=LED_GREEN;

}

else if(temp>=20 && temp<25){

statusMsg="COOL";
activeLED=LED_YELLOW;

}

else if(temp>=25 && temp<30){

statusMsg="WARM";
activeLED=LED_YELLOW;

}

else if(temp>=30 && temp<=35){

statusMsg="HOT";
activeLED=LED_RED;

}

else{

statusMsg="TOO HOT";
activeLED=LED_RED;

}


// ===== OLED =====

display.clearDisplay();

display.setTextSize(1);
display.setTextColor(SSD1306_WHITE);

display.setCursor(0,0);
display.printf("Temp: %.1f C",temp);

display.setCursor(0,12);
display.printf("Humi: %.1f %%",humi);

display.setTextSize(2);

display.setCursor(0,35);
display.print(statusMsg);

display.display();

}

}


// ===== LED nháy =====

if(now-prevLED>=intervalLED){

prevLED=now;

ledState=!ledState;

digitalWrite(activeLED,ledState);

}

}