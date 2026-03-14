// 1. CẤU HÌNH BLYNK CỦA BẠN (Phải để trên cùng)
#define BLYNK_TEMPLATE_ID "TMPL6FvqieFfo"
#define BLYNK_TEMPLATE_NAME "Blynk DHT"
#define BLYNK_AUTH_TOKEN "7fUhrV7HW0_1ff8haEZus3fQt3fHmwXE"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// 2. CẤU HÌNH WIFI (Mặc định cho mô phỏng Wokwi)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// 3. ĐỊNH NGHĨA CHÂN PIN (Khớp chính xác với diagram.json của bạn)
#define DHTPIN 16         // Cảm biến DHT22 nối GPIO 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN 21        // Đèn LED nối GPIO 21
#define BUTTON_PIN 23     // Nút nhấn nối GPIO 23

#define CLK 18            // Màn hình TM1637 (CLK) nối GPIO 18
#define DIO 19            // Màn hình TM1637 (DIO) nối GPIO 19
TM1637Display display(CLK, DIO);

// 4. BIẾN TOÀN CỤC & TIMER
BlynkTimer timer;
int runTime = 0;
bool ledState = false;

// --- Gửi dữ liệu cảm biến lên Blynk (V1, V2) ---
void sendSensor()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (!isnan(humidity) && !isnan(temperature))
  {
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);
  }
}

// --- Cập nhật thời gian hoạt động (V0) và màn hình ---
void sendUptime()
{
  runTime++;
  Blynk.virtualWrite(V0, runTime);
  
  // Hiển thị lên màn hình LED 4 số
  display.showNumberDec(runTime, true);
}

// --- Nhận lệnh điều khiển LED từ Web/App Blynk (V3) ---
BLYNK_WRITE(V3)
{
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

// --- Kiểm tra nút bấm vật lý (Đã sửa lỗi treo mạch) ---
void checkButton()
{
  static int lastButtonState = HIGH; 
  int currentButtonState = digitalRead(BUTTON_PIN);

  // Chỉ kích hoạt khi phát hiện nút VỪA ĐƯỢC NHẤN XUỐNG (Từ HIGH -> LOW)
  if (lastButtonState == HIGH && currentButtonState == LOW)
  {
    ledState = !ledState;               // Đảo trạng thái (Đang bật thì tắt, đang tắt thì bật)
    digitalWrite(LED_PIN, ledState);    // Điều khiển đèn thật
    Blynk.virtualWrite(V3, ledState);   // Đồng bộ trạng thái mới lên Blynk
  }
  
  lastButtonState = currentButtonState; // Cập nhật trạng thái
}

void setup()
{
  Serial.begin(115200);

  // Cài đặt chân In/Out
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Bắt buộc phải có PULLUP để chống nhiễu

  // Khởi động màn hình và cảm biến
  display.setBrightness(7);
  display.clear();
  dht.begin();

  // Khởi động Blynk
  Serial.println("Đang kết nối WiFi và Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Đã kết nối thành công!");

  // Cài đặt Timer chạy đa nhiệm (Tuyệt đối không dùng hàm delay)
  timer.setInterval(1000L, sendUptime);   // Mỗi 1 giây đếm thời gian
  timer.setInterval(2000L, sendSensor);   // Mỗi 2 giây đọc DHT22
  timer.setInterval(50L, checkButton);    // Mỗi 50 mili-giây quét nút bấm (Rất mượt)
}

void loop()
{
  Blynk.run();
  timer.run();
}