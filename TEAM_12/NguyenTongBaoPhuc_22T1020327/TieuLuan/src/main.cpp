#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ── WiFi & MQTT ──────────────────────────────────────
const char* WIFI_SSID   = "Wokwi-GUEST";
const char* WIFI_PASS   = "";
const char* MQTT_SERVER = "test.mosquitto.org";
const int   MQTT_PORT   = 1883;
const char* CLIENT_ID   = "esp32-traffic-001";
const char* TOPIC_CMD   = "traffic/led/control";
const char* TOPIC_STAT  = "traffic/led/status";

// ── GPIO ─────────────────────────────────────────────
#define PIN_RED    25
#define PIN_YELLOW 26
#define PIN_GREEN  27

// ── Trạng thái ───────────────────────────────────────
bool autoMode = true;
int  autoPhase = 0;      // 0=RED 1=YELLOW 2=GREEN
unsigned long lastChange = 0;
const unsigned long phaseDur[] = { 5000, 2000, 5000 };
const char* phaseNames[]       = { "RED", "YELLOW", "GREEN" };

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

// ── Hàm LED ──────────────────────────────────────────
void setLED(bool r, bool y, bool g) {
  digitalWrite(PIN_RED,    r ? HIGH : LOW);
  digitalWrite(PIN_YELLOW, y ? HIGH : LOW);
  digitalWrite(PIN_GREEN,  g ? HIGH : LOW);
}

void activatePhase(int phase) {
  if      (phase == 0) setLED(1,0,0);
  else if (phase == 1) setLED(0,1,0);
  else                 setLED(0,0,1);
  mqtt.publish(TOPIC_STAT, phaseNames[phase], true);
  Serial.printf("[LED] %s\n", phaseNames[phase]);
}

// ── Callback MQTT ─────────────────────────────────────
void onMessage(char* topic, byte* payload, unsigned int len) {
  String msg = "";
  for (unsigned int i=0; i<len; i++) msg += (char)payload[i];
  Serial.printf("[MQTT] Received: %s\n", msg.c_str());

  int cmd = msg.toInt();   // 0=AUTO 1=RED 2=YELLOW 3=GREEN
  if (cmd == 0) { autoMode = true; lastChange = millis(); return; }
  autoMode = false;
  if      (cmd == 1) activatePhase(0);
  else if (cmd == 2) activatePhase(1);
  else if (cmd == 3) activatePhase(2);
}

// ── WiFi Connect ─────────────────────────────────────
void connectWiFi() {
  Serial.print("[WiFi] Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print('.'); }
  Serial.printf("\n[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
}

// ── MQTT Connect ─────────────────────────────────────
void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Connecting...");
    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" OK");
      mqtt.subscribe(TOPIC_CMD);
      mqtt.publish(TOPIC_STAT, "READY");
    } else {
      Serial.printf(" Failed rc=%d, retry 3s\n", mqtt.state());
      delay(3000);
    }
  }
}

// ── Setup ────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);  
  Serial.println("=== BOOT ==="); 
  pinMode(PIN_RED, OUTPUT); pinMode(PIN_YELLOW, OUTPUT); pinMode(PIN_GREEN, OUTPUT);
  setLED(0,0,0);
  connectWiFi();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(onMessage);
  connectMQTT();
  lastChange = millis();
  activatePhase(0);   // Bắt đầu pha ĐỎ
}

// ── Loop ─────────────────────────────────────────────
void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();
  if (autoMode) {
    if (millis() - lastChange >= phaseDur[autoPhase]) {
      autoPhase = (autoPhase + 1) % 3;
      lastChange = millis();
      activatePhase(autoPhase);
    }
  }
}