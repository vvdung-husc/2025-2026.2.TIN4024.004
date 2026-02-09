#include <Arduino.h>

// Pin Definitions
#define GREEN_PIN  32
#define YELLOW_PIN 33
#define RED_PIN    25

// Timing Definitions (in milliseconds)
#define GREEN_TIME  7000 
#define YELLOW_TIME 3000
#define RED_TIME    6000 

// Blink speed settings (250ms ON + 250ms OFF = 2 blinks per second)
#define BLINK_DELAY 250 

void setup() {
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  
  // Initialize Serial
  Serial.begin(115200);
}

// Helper function to handle the blinking loop for a specific pin and duration
void blinkLight(int pin, int durationMs, String colorName) {
  Serial.print("Blinking [");
  Serial.print(colorName);
  Serial.print("] for ");
  Serial.print(durationMs / 1000);
  Serial.println(" seconds.");

  unsigned long startTime = millis();

  // Keep blinking until the duration has passed
  while (millis() - startTime < durationMs) {
    digitalWrite(pin, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(pin, LOW);
    delay(BLINK_DELAY);
  }
}

void loop() {
  // Ensure all are off before starting a phase
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(RED_PIN, LOW);

  // --- Phase 1: GREEN (7 Seconds Blinking) ---
  blinkLight(GREEN_PIN, GREEN_TIME, "GREEN");

  // --- Phase 2: YELLOW (3 Seconds Blinking) ---
  blinkLight(YELLOW_PIN, YELLOW_TIME, "YELLOW");

  // --- Phase 3: RED (6 Seconds Blinking) ---
  blinkLight(RED_PIN, RED_TIME, "RED");
}