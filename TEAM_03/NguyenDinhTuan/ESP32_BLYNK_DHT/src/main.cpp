// Blynk credentials
#define BLYNK_TEMPLATE_ID "TMPL6lGFdsfCS"
#define BLYNK_TEMPLATE_NAME "IOT TEMPLATE"
#define BLYNK_AUTH_TOKEN "a0fkBDgZbPUZbI0fxfE88yZpr1X2YKUc"

// WiFi credentialss
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Libraries
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// Pin definitions
#define DHT_PIN 16
#define LED_PIN 21
#define BUTTON_PIN 23
#define TM1637_CLK 18
#define TM1637_DIO 19

// DHT sensor setup
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// TM1637 display setup
TM1637Display display(TM1637_CLK, TM1637_DIO);

// Variables
float temperature = 0.0;
float humidity = 0.0;
bool displayEnabled = false; // Display and LED start OFF by default
bool buttonState = false;
bool lastButtonState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long startTime = 0;
unsigned long uptimeSeconds = 0;
unsigned long sensorReadInterval = 2000; // Read sensor every 2 seconds
unsigned long blynkUpdateInterval = 5000; // Update temp/humidity every 5 seconds
String deviceName = "ESP32-TempHumid";

BlynkTimer timer;

// Forward declarations
void readSensorData();
void sendDataToBlynk();
void sendUptimeToBlynk();
void updateDisplay();
void calculateUptime();
void checkButton();
void updateLED();

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW); // LED starts OFF
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize TM1637 display
  display.setBrightness(0x0a); // Medium brightness
  display.clear();
  
  // Initialize Blynk
  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Record start time
  startTime = millis();
  
  // Setup timers
  timer.setInterval(sensorReadInterval, readSensorData);
  timer.setInterval(blynkUpdateInterval, sendDataToBlynk);
  timer.setInterval(1000L, updateDisplay); // Update display every second
  timer.setInterval(1000L, calculateUptime); // Calculate uptime every second
  timer.setInterval(1000L, sendUptimeToBlynk); 
  
  Serial.println("System initialized!");
  Serial.println("Display and LED are OFF by default.");
  Serial.println("Control via Blynk V1 or press physical button.");
  
  // Send initial device name and display state
  Blynk.virtualWrite(V4, deviceName);
  Blynk.virtualWrite(V1, 0); // Display and LED OFF by default
}

void loop() {
  Blynk.run();
  timer.run();
  checkButton();
}

// Read temperature and humidity from DHT22
void readSensorData() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  // Check if readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Temperature and humidity read successfully (no serial output)
}

// Send temperature and humidity data to Blynk
void sendDataToBlynk() {
  // Send display state
  Blynk.virtualWrite(V1, displayEnabled ? 1 : 0);
  
  // Send temperature
  Blynk.virtualWrite(V2, temperature);
  
  // Send humidity
  Blynk.virtualWrite(V3, humidity);
  
  // Data sent silently (no serial output)
}

// Send uptime in REAL-TIME (called every second)
void sendUptimeToBlynk() {
  Blynk.virtualWrite(V0, uptimeSeconds);
}

// Calculate uptime in seconds
void calculateUptime() {
  uptimeSeconds = (millis() - startTime) / 1000;
}

// Update LED to match display state
void updateLED() {
  digitalWrite(LED_PIN, displayEnabled ? HIGH : LOW);
}

// Check button press to toggle display and LED
void checkButton() {
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == LOW) { // Button pressed (active low)
        displayEnabled = !displayEnabled; // Toggle display and LED
        updateLED();
        
        // Update Blynk to sync
        Blynk.virtualWrite(V1, displayEnabled ? 1 : 0);
        
        Serial.print("Button pressed: Display and LED ");
        Serial.println(displayEnabled ? "ON" : "OFF");
        
        // Update display immediately
        if (displayEnabled) {
          display.setBrightness(0x0a);
        } else {
          display.clear();
        }
      }
    }
  }
  
  lastButtonState = reading;
}

// Update TM1637 display with uptime (only if enabled)
void updateDisplay() {
  if (!displayEnabled) {
    // Display is disabled - clear and turn off
    display.clear();
    return;
  }
  
  // Display is enabled - show uptime in different formats based on duration
  if (uptimeSeconds < 10000) {
    // Display seconds directly (0-9999)
    display.showNumberDec(uptimeSeconds, false);
  } else if (uptimeSeconds < 600000) {
    // Display minutes after 10000 seconds (shows as minutes up to 9999 min)
    unsigned long minutes = uptimeSeconds / 60;
    display.showNumberDec(minutes, false);
  } else {
    // Display hours after 600000 seconds
    unsigned long hours = uptimeSeconds / 3600;
    display.showNumberDec(hours, false);
  }
}

// Blynk virtual pin handlers

// V0 - Uptime in seconds (read-only from device)
// Sent in REAL-TIME every 1 second

// V1 - Display & LED Control (TM1637 and LED ON/OFF)
BLYNK_WRITE(V1) {
  int value = param.asInt();
  displayEnabled = (value == 1);
  
  // Update LED to match display state
  updateLED();
  
  if (displayEnabled) {
    Serial.println("Blynk V1: Display and LED ON");
    display.setBrightness(0x0a); // Set brightness when enabled
  } else {
    Serial.println("Blynk V1: Display and LED OFF");
    display.clear(); // Clear display immediately when disabled
  }
}

// V2 - Temperature (read-only from device)
// Automatically sent every 5 seconds

// V3 - Humidity (read-only from device)
// Automatically sent every 5 seconds

// V4 - Device Name
BLYNK_WRITE(V4) {
  deviceName = param.asString();
  Serial.print("Device name updated to: ");
  Serial.println(deviceName);
}

// Connected to Blynk
BLYNK_CONNECTED() {
  Serial.println("Connected to Blynk!");
  
  // Sync display control state from server
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V4);
  
  // Send initial temperature and humidity data
  sendDataToBlynk();
  
  // Uptime will be sent automatically every second by timer
}