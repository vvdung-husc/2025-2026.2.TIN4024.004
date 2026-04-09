#define BLYNK_TEMPLATE_ID "TMPL6zoKN_usl"
#define BLYNK_TEMPLATE_NAME "ESP32 WEB SERVER"
#define BLYNK_AUTH_TOKEN "m6AxsNll-e3XFjJoTNUW0O551R1_qWSm"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

#define DHTPIN 14
#define LDR_PIN 34
#define CLK 22
#define DIO 23
#define LED_RED 18
#define LED_YELLOW 5
#define LED_GREEN 17
#define LED_BLUE 12

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

DHT dht(DHTPIN, DHT22);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

float lastT = -1.0, lastH = -1.0;
int lastLDR = -1;
bool isDark = false;

BLYNK_WRITE(V1) {
  digitalWrite(LED_BLUE, param.asInt());
}

void updateSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int ldrVal = analogRead(LDR_PIN);

  if (isnan(h) || isnan(t)) return;

  // CHỈ CẬP NHẬT NHIỆT ĐỘ NẾU THAY ĐỔI
  if (abs(t - lastT) >= 0.5) {
    Blynk.virtualWrite(V2, t);
    Serial.print("Nhiệt độ: "); Serial.print(t, 1); Serial.println("°C");
    lastT = t;
  }

  // CHỈ CẬP NHẬT ĐỘ ẨM NẾU THAY ĐỔI
  if (abs(h - lastH) >= 1.0) {
    Blynk.virtualWrite(V3, h);
    Serial.print("Độ ẩm: "); Serial.print(h, 1); Serial.println("%");
    lastH = h;
  }

  // CHỈ CẬP NHẬT ÁNH SÁNG NẾU THAY ĐỔI
  if (abs(ldrVal - lastLDR) >= 50) {
    Blynk.virtualWrite(V4, ldrVal);
    Serial.print("Ánh sáng LDR: "); Serial.println(ldrVal);
    lastLDR = ldrVal;
    isDark = (ldrVal > 2000);
  }
}

void blinkLight(int pin, int seconds) {
  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i);
    Blynk.virtualWrite(V0, i);

    // Nhịp 500ms Tắt -> 500ms Mở
    digitalWrite(pin, LOW);
    delay(500);
    digitalWrite(pin, HIGH);
    delay(500);
    
    Blynk.run();
    if (analogRead(LDR_PIN) > 2000) return;
  }
  digitalWrite(pin, LOW);
}

void trafficLogic() {
  if (!isDark) {
    blinkLight(LED_GREEN, 7);
    blinkLight(LED_YELLOW, 3);
    blinkLight(LED_RED, 10);
  } else {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    display.showNumberDec(88);
    Blynk.virtualWrite(V0, 0);
    
    digitalWrite(LED_YELLOW, LOW); delay(500);
    digitalWrite(LED_YELLOW, HIGH); delay(500);
    Blynk.run();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  display.setBrightness(0x0f);
  dht.begin();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());
  
  Blynk.config(auth);
  
  // Hiển thị đầy đủ thông tin lần đầu khi khởi động
  Serial.println("Thông số ban đầu");
  updateSensors(); 

  timer.setInterval(2000L, updateSensors);
}

void loop() {
  Blynk.run();
  timer.run();
  trafficLogic();
}