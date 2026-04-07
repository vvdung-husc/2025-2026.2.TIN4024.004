#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <DHTesp.h>
#include <UniversalTelegramBot.h>

// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= TELEGRAM =================
const char* BOT_TOKEN = "8220949647:AAFtxHjJhEHOv1EfMaxdM6fLpB47lhIPOVI";
const char* AUTHORIZED_CHAT_ID = "6977629641";

// ================= PIN =================
#define WS2812_PIN 5
#define NUM_LEDS 16
#define NORMAL_LED_PIN 2
#define DHT_PIN 4
#define MQ2_PIN 34
#define SDA_PIN 21
#define SCL_PIN 22

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

WebServer server(80);
WiFiClientSecure telegramClient;
UniversalTelegramBot bot(BOT_TOKEN, telegramClient);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_NeoPixel strip(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);
DHTesp dhtSensor;

// ================= STATE =================
struct LedState {
  bool power = true;
  uint8_t brightness = 150;
  uint8_t r = 255;
  uint8_t g = 255;
  uint8_t b = 255;
  String effect = "static";
} state;

float temperatureC = 0.0f;
float humidity = 0.0f;
int gasValue = 0;

unsigned long lastSensorRead = 0;
unsigned long lastOledUpdate = 0;
unsigned long lastEffectTime = 0;
unsigned long lastTelegramCheck = 0;

const unsigned long sensorInterval = 2000;
const unsigned long oledInterval = 200;
const unsigned long telegramInterval = 1200;

bool blinkState = true;
uint16_t rainbowHue = 0;
int breatheValue = 5;
int breatheStep = 5;
int theaterOffset = 0;

// ================= HELPER =================
uint8_t clampByte(int value) {
  if (value < 0) return 0;
  if (value > 255) return 255;
  return (uint8_t)value;
}

String currentColorName() {
  if (state.r == 255 && state.g == 0 && state.b == 0) return "RED";
  if (state.r == 0 && state.g == 255 && state.b == 0) return "GREEN";
  if (state.r == 0 && state.g == 0 && state.b == 255) return "BLUE";
  if (state.r == 255 && state.g == 255 && state.b == 255) return "WHITE";
  return "CUSTOM";
}

String getHexColor() {
  char buf[8];
  sprintf(buf, "%02X%02X%02X", state.r, state.g, state.b);
  return String(buf);
}

void turnOffStrip() {
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, 0);
  strip.setBrightness(0);
  strip.show();
}

void fillStrip(uint8_t r, uint8_t g, uint8_t b) {
  strip.setBrightness(state.power ? state.brightness : 0);
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void resetEffectVars() {
  blinkState = true;
  rainbowHue = 0;
  breatheValue = 5;
  breatheStep = 5;
  theaterOffset = 0;
  lastEffectTime = 0;
}

void applyCurrentState() {
  if (!state.power) {
    turnOffStrip();
    digitalWrite(NORMAL_LED_PIN, LOW);
    return;
  }

  digitalWrite(NORMAL_LED_PIN, HIGH);

  if (state.effect == "static") {
    fillStrip(state.r, state.g, state.b);
  }
}

// ================= EFFECT =================
void effectBlink() {
  if (millis() - lastEffectTime < 500) return;
  lastEffectTime = millis();
  blinkState = !blinkState;
  if (blinkState) fillStrip(state.r, state.g, state.b);
  else fillStrip(0, 0, 0);
}

void effectRainbow() {
  if (millis() - lastEffectTime < 20) return;
  lastEffectTime = millis();

  strip.setBrightness(state.power ? state.brightness : 0);
  for (int i = 0; i < NUM_LEDS; i++) {
    uint32_t pixelHue = rainbowHue + (i * 65536L / NUM_LEDS);
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  }
  strip.show();
  rainbowHue += 256;
}

void effectBreathe() {
  if (millis() - lastEffectTime < 25) return;
  lastEffectTime = millis();

  breatheValue += breatheStep;
  if (breatheValue >= state.brightness) {
    breatheValue = state.brightness;
    breatheStep = -abs(breatheStep);
  }
  if (breatheValue <= 5) {
    breatheValue = 5;
    breatheStep = abs(breatheStep);
  }

  strip.setBrightness(state.power ? breatheValue : 0);
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(state.r, state.g, state.b));
  }
  strip.show();
}

void effectTheater() {
  if (millis() - lastEffectTime < 120) return;
  lastEffectTime = millis();

  strip.setBrightness(state.power ? state.brightness : 0);
  for (int i = 0; i < NUM_LEDS; i++) {
    if ((i + theaterOffset) % 3 == 0) strip.setPixelColor(i, strip.Color(state.r, state.g, state.b));
    else strip.setPixelColor(i, 0);
  }
  strip.show();
  theaterOffset = (theaterOffset + 1) % 3;
}

// ================= SENSOR =================
void readSensors() {
  if (millis() - lastSensorRead < sensorInterval) return;
  lastSensorRead = millis();

  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  if (!isnan(data.temperature)) temperatureC = data.temperature;
  if (!isnan(data.humidity)) humidity = data.humidity;

  gasValue = analogRead(MQ2_PIN);
}

// ================= OLED =================
void updateOLED() {
  if (millis() - lastOledUpdate < oledInterval) return;
  lastOledUpdate = millis();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("TEMP:");
  display.print(temperatureC, 1);
  display.print("C");

  display.setCursor(0, 10);
  display.print("HUM:");
  display.print(humidity, 0);
  display.print("%");

  display.setCursor(0, 20);
  display.print("GAS:");
  display.print(gasValue);

  display.setTextSize(2);
  display.setCursor(0, 32);
  display.print(currentColorName());

  display.setTextSize(1);
  display.setCursor(0, 54);
  display.print("B:");
  display.print(state.brightness);
  display.print(" FX:");
  display.print(state.effect);

  display.display();
}

// ================= COMMAND =================
void handleCommand(String cmdRaw) {
  String cmd = cmdRaw;
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "on" || cmd == "/on") {
    state.power = true;
    state.effect = "static";
  } else if (cmd == "off" || cmd == "/off") {
    state.power = false;
  } else if (cmd == "red" || cmd == "/red") {
    state.power = true;
    state.effect = "static";
    state.r = 255; state.g = 0; state.b = 0;
  } else if (cmd == "green" || cmd == "/green") {
    state.power = true;
    state.effect = "static";
    state.r = 0; state.g = 255; state.b = 0;
  } else if (cmd == "blue" || cmd == "/blue") {
    state.power = true;
    state.effect = "static";
    state.r = 0; state.g = 0; state.b = 255;
  } else if (cmd == "white" || cmd == "/white") {
    state.power = true;
    state.effect = "static";
    state.r = 255; state.g = 255; state.b = 255;
  } else if (cmd == "rainbow" || cmd == "/rainbow") {
    state.power = true;
    state.effect = "rainbow";
  } else if (cmd == "blink" || cmd == "/blink") {
    state.power = true;
    state.effect = "blink";
  } else if (cmd == "breathe" || cmd == "/breathe") {
    state.power = true;
    state.effect = "breathe";
  } else if (cmd == "theater" || cmd == "/theater") {
    state.power = true;
    state.effect = "theater";
  }

  resetEffectVars();
  applyCurrentState();
}

// ================= TELEGRAM =================
void sendTelegramStatus(const String& chatId) {
  String msg;
  msg += "STATUS\n";
  msg += "Power: " + String(state.power ? "ON" : "OFF") + "\n";
  msg += "Brightness: " + String(state.brightness) + "\n";
  msg += "Color: " + currentColorName() + " (#" + getHexColor() + ")\n";
  msg += "Effect: " + state.effect;
  bot.sendMessage(chatId, msg, "");
}

void handleTelegramCommand(String text, String chat_id) {
  text.trim();
  text.toLowerCase();

  if (chat_id != String(AUTHORIZED_CHAT_ID)) {
    bot.sendMessage(chat_id, "Khong co quyen dieu khien.", "");
    return;
  }

  if (text == "/start") {
    String msg;
    msg += "/on /off\n";
    msg += "/red /green /blue /white\n";
    msg += "/rainbow /blink /breathe /theater\n";
    msg += "/brightness 0-255\n";
    msg += "/status";
    bot.sendMessage(chat_id, msg, "");
    return;
  }

  if (text == "/status") {
    sendTelegramStatus(chat_id);
    return;
  }

  if (text.startsWith("/brightness ")) {
    int value = text.substring(12).toInt();
    state.power = true;
    state.brightness = clampByte(value);
    resetEffectVars();
    applyCurrentState();
    bot.sendMessage(chat_id, "Brightness: " + String(state.brightness), "");
    return;
  }

  handleCommand(text);
  sendTelegramStatus(chat_id);
}

void checkTelegramMessages() {
  if (String(BOT_TOKEN) == "YOUR_BOT_TOKEN") return;
  if (String(AUTHORIZED_CHAT_ID) == "YOUR_CHAT_ID") return;
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastTelegramCheck < telegramInterval) return;

  lastTelegramCheck = millis();

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      handleTelegramCommand(bot.messages[i].text, bot.messages[i].chat_id);
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ================= WEB =================
String buildPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>ESP32 Smart LED</title>
  <style>
    body{font-family:Arial;background:#111827;color:#fff;text-align:center;padding:20px}
    .box{max-width:560px;margin:auto;background:#1f2937;padding:20px;border-radius:16px}
    button{margin:6px;padding:10px 14px;border:none;border-radius:10px;background:#374151;color:#fff;cursor:pointer}
    button:hover{background:#4b5563}
    input[type=range]{width:100%}
    .card{background:#0f172a;padding:10px;border-radius:12px;margin:10px 0}
    .row{margin:10px 0}
    .status{margin-top:10px;color:#93c5fd}
  </style>
</head>
<body>
  <div class="box">
    <h2>ESP32 Smart LED Control</h2>

    <div class="card">
      <div>Color: <span id="colorName">--</span></div>
      <div>Brightness: <span id="bval">--</span></div>
      <div>Effect: <span id="fx">--</span></div>
      <div>Temp: <span id="temp">--</span> C</div>
      <div>Hum: <span id="hum">--</span> %</div>
      <div>Gas: <span id="gas">--</span></div>
    </div>

    <div class="row">
      <input id="color" type="color" value="#FFFFFF">
    </div>

    <div class="row">
      <input id="brightness" type="range" min="0" max="255" value="150">
    </div>

    <div class="row">
      <button onclick="sendCmd('/on')">ON</button>
      <button onclick="sendCmd('/off')">OFF</button>
      <button onclick="sendCmd('/red')">RED</button>
      <button onclick="sendCmd('/green')">GREEN</button>
      <button onclick="sendCmd('/blue')">BLUE</button>
      <button onclick="sendCmd('/white')">WHITE</button>
    </div>

    <div class="row">
      <button onclick="sendCmd('/blink')">BLINK</button>
      <button onclick="sendCmd('/rainbow')">RAINBOW</button>
      <button onclick="sendCmd('/breathe')">BREATHE</button>
      <button onclick="sendCmd('/theater')">THEATER</button>
    </div>

    <div id="msg" class="status"></div>
  </div>

<script>
async function callApi(url) {
  const res = await fetch(url);
  const text = await res.text();
  document.getElementById('msg').innerText = text;
  await updateData();
}

async function sendCmd(cmd){
  await callApi('/voice?cmd=' + encodeURIComponent(cmd));
}

async function updateData(){
  const res = await fetch('/status');
  const data = await res.json();
  document.getElementById('temp').innerText = data.temperature;
  document.getElementById('hum').innerText = data.humidity;
  document.getElementById('gas').innerText = data.gas;
  document.getElementById('bval').innerText = data.brightness;
  document.getElementById('colorName').innerText = data.colorName;
  document.getElementById('fx').innerText = data.effect;
  document.getElementById('brightness').value = data.brightness;
  document.getElementById('color').value = data.hex;
}

document.getElementById('brightness').addEventListener('change', async (e) => {
  await callApi('/brightness?b=' + e.target.value);
});

document.getElementById('color').addEventListener('change', async (e) => {
  await callApi('/color?c=' + e.target.value.replace('#',''));
});

setInterval(updateData, 1200);
updateData();
</script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", buildPage());
}

void handleStatus() {
  String json = "{";
  json += "\"temperature\":" + String(temperatureC, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"gas\":" + String(gasValue) + ",";
  json += "\"brightness\":" + String(state.brightness) + ",";
  json += "\"power\":" + String(state.power ? "true" : "false") + ",";
  json += "\"effect\":\"" + state.effect + "\",";
  json += "\"colorName\":\"" + currentColorName() + "\",";
  json += "\"hex\":\"#" + getHexColor() + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void handleColor() {
  if (!server.hasArg("c")) {
    server.send(400, "text/plain", "Missing color");
    return;
  }

  String c = server.arg("c");
  if (c.length() != 6) {
    server.send(400, "text/plain", "Invalid color");
    return;
  }

  state.r = strtol(c.substring(0, 2).c_str(), NULL, 16);
  state.g = strtol(c.substring(2, 4).c_str(), NULL, 16);
  state.b = strtol(c.substring(4, 6).c_str(), NULL, 16);
  state.power = true;
  state.effect = "static";

  resetEffectVars();
  applyCurrentState();
  server.send(200, "text/plain", "Color updated");
}

void handleBrightness() {
  if (!server.hasArg("b")) {
    server.send(400, "text/plain", "Missing brightness");
    return;
  }

  state.power = true;
  state.brightness = clampByte(server.arg("b").toInt());
  resetEffectVars();
  applyCurrentState();
  server.send(200, "text/plain", "Brightness updated");
}

void handleVoice() {
  if (!server.hasArg("cmd")) {
    server.send(400, "text/plain", "Missing cmd");
    return;
  }

  handleCommand(server.arg("cmd"));
  server.send(200, "text/plain", "Command sent");
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  pinMode(NORMAL_LED_PIN, OUTPUT);
  digitalWrite(NORMAL_LED_PIN, LOW);

  strip.begin();
  strip.clear();
  strip.show();

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  connectWiFi();
  telegramClient.setInsecure();

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/color", handleColor);
  server.on("/brightness", handleBrightness);
  server.on("/voice", handleVoice);
  server.begin();

  applyCurrentState();

  if (WiFi.status() == WL_CONNECTED &&
      String(BOT_TOKEN) != "YOUR_BOT_TOKEN" &&
      String(AUTHORIZED_CHAT_ID) != "YOUR_CHAT_ID") {
    bot.sendMessage(AUTHORIZED_CHAT_ID, "ESP32 online", "");
  }
}

void loop() {
  server.handleClient();
  readSensors();
  updateOLED();
  checkTelegramMessages();

  if (!state.power) return;

  if (state.effect == "blink") effectBlink();
  else if (state.effect == "rainbow") effectRainbow();
  else if (state.effect == "breathe") effectBreathe();
  else if (state.effect == "theater") effectTheater();
}