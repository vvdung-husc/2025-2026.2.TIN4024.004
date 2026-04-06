#define BLYNK_TEMPLATE_ID "TMPL6ORkRKGpF"
#define BLYNK_TEMPLATE_NAME "ESP32BlynkCLKK"
#define BLYNK_AUTH_TOKEN "WufKHXaJx4WrtjkEX0JBARIaZ7seRzk1"

char auth[] = "WufKHXaJx4WrtjkEX0JBARIaZ7seRzk1";
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PMS.h>

#define MQ_PIN     32
#define LED_PIN    5
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
HardwareSerial pmsSerial(2);   // UART2
PMS pms(pmsSerial);
PMS::DATA data;

BlynkTimer timer;

float co_ppm = 0, pm25 = 0;

// ====================== ĐỌC MQ-135 (CO) ======================
void readMQ135() {
  int raw = analogRead(MQ_PIN);
  float voltage = raw * (3.3 / 4095.0);
  if (voltage < 0.1) voltage = 0.1;

  float RS = (3.3 / voltage - 1) * 10000.0;
  float ratio = RS / 10000.0;
  co_ppm = pow(10, ((log10(ratio) - 0.2) / -0.48));

  if (co_ppm < 0) co_ppm = 0;
  if (co_ppm > 1000) co_ppm = 1000;
}

// ====================== ĐỌC PMS5003 (PM2.5) ======================
void readPMS() {
  static unsigned long lastRead = 0;
  if (millis() - lastRead < 1000) return;   // Đọc PMS mỗi ~1 giây
  lastRead = millis();

  if (pms.read(data)) {
    pm25 = data.PM_AE_UG_2_5;
    Serial.println("→ PMS5003 đọc thành công!");
  } else {
    // Nếu chưa đọc được, simulate để test
    pm25 = 15 + random(0, 45);   // Giá trị giả để thấy thay đổi
    //Serial.println("→ PMS5003 chưa đọc được, đang simulate...");
  }
}

// ====================== HIỂN THỊ TRÊN SERIAL MONITOR ======================
void printToSerial() {
  readMQ135();
  readPMS();

  Serial.print("CO   : ");
  Serial.print(co_ppm, 0);
  Serial.print(" ppm  |  PM2.5 : ");
  Serial.print(pm25, 0);
  Serial.println(" µg/m³");
}

// ====================== GỬI DỮ LIỆU LÊN BLYNK + OLED ======================
void updateDisplay() {
  readMQ135();
  readPMS();

  Blynk.virtualWrite(V0, millis() / 1000);
  Blynk.virtualWrite(V2, (int)co_ppm);
  Blynk.virtualWrite(V3, (int)pm25);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(" Air Quality Monitor");
  display.println("--------------------");
  display.print("CO   : "); display.print(co_ppm, 0); display.println(" ppm");
  display.print("PM2.5: "); display.print(pm25, 0); display.println(" ug/m3");
  display.display();
}

// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);
  pmsSerial.begin(9600, SERIAL_8N1, 17, 16);  // RX=17, TX=16

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed!");
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Khoi dong...");
  display.display();

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(2000L, updateDisplay);     // OLED + Blynk
  timer.setInterval(2500L, printToSerial);     // Serial Monitor

  Serial.println("\n Đã kết nối...");
  Serial.println("Đang đọc dữ liệu từ MQ-135 và PMS5003...");
}

// ====================== BLYNK ======================
BLYNK_WRITE(V1) {
  digitalWrite(LED_PIN, param.asInt());
}

void loop() {
  Blynk.run();
  timer.run();
}