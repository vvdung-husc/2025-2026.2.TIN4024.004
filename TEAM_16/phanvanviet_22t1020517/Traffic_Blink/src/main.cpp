#include <Arduino.h>

const int ledXanh = 25;
const int ledVang = 27;
const int ledDo = 26;
const int buttonPin = 23; 
const int ledBlue = 21;   

const int CLK = 33;
const int DIO = 32;

const uint8_t DIGITS[] = { 
  0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f 
};

void tm1637_start() {
  digitalWrite(DIO, LOW); delayMicroseconds(100);
  digitalWrite(CLK, LOW); delayMicroseconds(100);
}

void tm1637_stop() {
  digitalWrite(DIO, LOW); delayMicroseconds(100);
  digitalWrite(CLK, HIGH); delayMicroseconds(100);
  digitalWrite(DIO, HIGH); delayMicroseconds(100);
}

void tm1637_writeByte(uint8_t b) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK, LOW);
    if (b & 1) digitalWrite(DIO, HIGH);
    else digitalWrite(DIO, LOW);
    delayMicroseconds(100);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(100);
    b >>= 1;
  }
  digitalWrite(CLK, LOW); digitalWrite(DIO, LOW); 
  digitalWrite(CLK, HIGH); digitalWrite(CLK, LOW);
}

void hienThiSo(int num) {
  digitalWrite(ledBlue, HIGH); 

  tm1637_start(); tm1637_writeByte(0x40); tm1637_stop();

  tm1637_start(); tm1637_writeByte(0x8F); tm1637_stop();

  int chuc = num / 10;
  int donvi = num % 10;

  tm1637_start();
  tm1637_writeByte(0xC0);
  
  tm1637_writeByte(0x00);
  tm1637_writeByte(0x00);
  tm1637_writeByte(DIGITS[chuc]);
  tm1637_writeByte(DIGITS[donvi]);
  
  tm1637_stop();
}

void setup() {
  pinMode(ledXanh, OUTPUT); pinMode(ledVang, OUTPUT); pinMode(ledDo, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(ledBlue, OUTPUT);
  
  pinMode(CLK, OUTPUT); pinMode(DIO, OUTPUT);
  digitalWrite(CLK, HIGH); digitalWrite(DIO, HIGH);

  Serial.begin(115200);
  Serial.println("--- SYSTEM START ---");
}

void chayPhaDen(int chanDen, int thoiGian) {
  digitalWrite(ledXanh, LOW); digitalWrite(ledVang, LOW); digitalWrite(ledDo, LOW);
  digitalWrite(chanDen, HIGH);

  for (int i = thoiGian; i >= 0; i--) {
    hienThiSo(i);
    delay(1000);
  }
}

void loop() {
  chayPhaDen(ledXanh, 7);
  chayPhaDen(ledVang, 3);
  chayPhaDen(ledDo, 10);
}