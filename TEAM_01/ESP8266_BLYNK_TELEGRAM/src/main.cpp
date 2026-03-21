/*
Thông tin nhóm 01
1. Hà Văn Hòa
2. Trương Công Bin

 */
#define BLYNK_TEMPLATE_ID "TMPL6Fvb3xPaK"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <cmath>

void readSensors();
void updateBlynk();
void updateOLED();
void handleTelegram();
void checkAndNotifyTelegram();

#include <DHT.h>
#define DHTPIN  12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_SDA       27
#define OLED_SCL       26
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define MQ2_PIN  32

#define LED_PIN  5

#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASS     = "";

const char* BLYNK_AUTH    = "Yr8afoweFuTp9fsb23CwzM5sRE_jBRdQ"; // Thay Token 

const char* BOT_TOKEN     = "8620972355:AAEjOrBF0xD2Szth__yMPEXO_xAleF6uYAI";
const String ALLOWED_CHAT = "-1003390098512";   

const String TEAM_NAME    = "Team 01.004";

#define TEMP_THRESHOLD  0.5   // do C
#define HUM_THRESHOLD   1.0   // %
// ════════════════════════════════════════════════════════════

// ── Bien toan cuc ──────────────────────────────────────────
float  temperature    = 0, lastSentTemp = -999;
float  humidity       = 0, lastSentHum  = -999;
int    gasRaw         = 0;   // ADC 0-4095
bool   ledState       = false;
unsigned long uptimeSeconds = 0;

unsigned long lastSensorRead   = 0;
unsigned long lastBlynkUpdate  = 0;
unsigned long lastOledUpdate   = 0;
unsigned long lastTelegramPoll = 0;
unsigned long lastUptimeTick   = 0;

const unsigned long SENSOR_INTERVAL  = 2000;
const unsigned long BLYNK_INTERVAL   = 3000;
const unsigned long OLED_INTERVAL    = 1000;
const unsigned long TELEGRAM_POLL    = 2000;

WiFiClientSecure    secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);


void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED khong tim thay!");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32 IoT");
    display.println(TEAM_NAME);
    display.println("Dang khoi dong...");
    display.display();
  }

  dht.begin();
  analogReadResolution(12);

  // WiFi
  Serial.print("Ket noi WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500); Serial.print("."); attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi THAT BAI!");
  }

  