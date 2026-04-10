#define BLYNK_TEMPLATE_ID "TMPL665DqNu9i"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "E6XSMMAkwcauDABxiG8j1x4Byf33gnla"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <time.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== WIFI =====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== NTP =====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;

// ===== DHT =====
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== TIMER =====
BlynkTimer timer;

// ===== HÀM TRẠNG THÁI =====
String getWeatherStatus(float temp) {
  if (temp < 13) return "RAT LANH";
  else if (temp < 20) return "LANH";
  else if (temp < 25) return "MAT";
  else if (temp < 30) return "AM";
  else if (temp < 35) return "NONG";
  else return "RAT NONG";
}

// ===== HÀM GỬI DỮ LIỆU =====
void sendData() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) return;

  String status = getWeatherStatus(temp);

  // ===== LẤY THỜI GIAN =====
  struct tm timeinfo;
  char timeStr[20] = "--:--:--";

  if (getLocalTime(&timeinfo)) {
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  }

  // ===== GỬI LÊN BLYNK =====
  Blynk.virtualWrite(V0, timeStr); // TIMENOW
  Blynk.virtualWrite(V1, status);  // STATUS
  Blynk.virtualWrite(V2, temp);    // TEMPERATURE
  Blynk.virtualWrite(V3, hum);     // HUMIDITY

  // ===== OLED FORMATTING =====
  display.clearDisplay();

  // 1. Vẽ khung ngoài và các đường phân cách
  display.drawRect(0, 0, 128, 64, WHITE);
  display.drawLine(0, 16, 128, 16, WHITE); // Dòng kẻ ngang 1
  display.drawLine(0, 44, 128, 44, WHITE); // Dòng kẻ ngang 2
  display.drawLine(64, 44, 64, 64, WHITE); // Dòng kẻ dọc chia đôi phần dưới

  // 2. HEADER: THỜI GIAN (Căn giữa tuyệt đối)
  display.setTextSize(1);
  // 8 ký tự * 6 pixel = 48 pixel. Căn giữa: (128 - 48)/2 = 40
  display.setCursor(40, 5); 
  display.print(timeStr);

  // 3. MIDDLE: NHIỆT ĐỘ (Size to, có ký hiệu độ C)
  display.setTextSize(2);
  String tempStr = String(temp, 1);
  
  // Tính toán để căn giữa cả khối "XX.X °C"
  int tempLen = tempStr.length() * 12; // Chiều rộng chữ số
  int totalWidth = tempLen + 22;       // Cộng thêm khoảng cách và chữ C
  int startX = (128 - totalWidth) / 2;

  display.setCursor(startX, 23);
  display.print(tempStr);
  
  // Vẽ dấu độ (°) và chữ C
  display.drawCircle(startX + tempLen + 5, 24, 3, WHITE); // Vòng tròn nhỏ làm dấu độ
  display.setCursor(startX + tempLen + 12, 23);
  display.print("C");

  // 4. BOTTOM LEFT: ĐỘ ẨM
  display.setTextSize(1);
  display.setCursor(6, 51);
  display.print("Hum:");
  display.print(hum, 0);
  display.print("%");

  // 5. BOTTOM RIGHT: TRẠNG THÁI (Căn giữa ô bên phải)
  int statusWidth = status.length() * 6; // Chiều rộng text status
  int statusX = 64 + (64 - statusWidth) / 2; // Căn giữa trong khoảng 64px bên phải
  display.setCursor(statusX, 51);
  display.print(status);

  display.display();

  // Debug Serial
  Serial.println("======");
  Serial.println(timeStr);
  Serial.println(temp);
  Serial.println(hum);
  Serial.println(status);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  dht.begin();

  // Khai báo chân I2C tùy chỉnh (SDA = 13, SCL = 12)
  Wire.begin(13, 12); 

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED lỗi!");
    while (true);
  }
  display.setTextColor(WHITE);

  // WiFi + Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // NTP
  configTime(gmtOffset_sec, 0, ntpServer);

  // Timer
  timer.setInterval(2000L, sendData);
}

// ===== LOOP =====
void loop() {
  Blynk.run();
  timer.run();
}