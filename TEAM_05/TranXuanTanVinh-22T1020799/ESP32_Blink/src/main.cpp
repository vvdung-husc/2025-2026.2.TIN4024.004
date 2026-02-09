#include <Arduino.h>

#define LED_RED     25
#define LED_YELLOW  33
#define LED_GREEN   32

const unsigned long RED_TIME    = 5000; 
const unsigned long YELLOW_TIME = 3000; 
const unsigned long GREEN_TIME  = 7000; 

unsigned long previousMillis = 0;  
unsigned long interval = RED_TIME; 

enum TrafficState {
  STATE_RED,
  STATE_YELLOW,
  STATE_GREEN
};

TrafficState currentState = STATE_RED; 

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  
  Serial.println("LED [RED   ] ON => 5 Seconds");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    switch (currentState) {
      
      case STATE_RED:
        currentState = STATE_GREEN;
        interval = GREEN_TIME;
        
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_GREEN, HIGH);
        
        Serial.println("LED [GREEN ] ON => 7 Seconds");
        break;

      case STATE_GREEN:
        currentState = STATE_YELLOW;
        interval = YELLOW_TIME;
        
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(LED_GREEN, LOW);
        
        Serial.println("LED [YELLOW] ON => 3 Seconds");
        break;

      case STATE_YELLOW:
        currentState = STATE_RED;
        interval = RED_TIME;
        
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_GREEN, LOW);
        
        Serial.println("LED [RED   ] ON => 5 Seconds");
        break;
    }
  }
}