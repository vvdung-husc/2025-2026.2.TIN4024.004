#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

/* Thông tin thiết bị từ Blynk của bạn */
#define BLYNK_TEMPLATE_ID "TMPL6M9I1e36f"
#define BLYNK_TEMPLATE_NAME "Blynk"
#define BLYNK_AUTH_TOKEN "6pvaXbXwIBNSmwHVhPbFtmVWFS3OJeqC"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

void updateSystemState();

// Cấu hình mạng WiFi cho Wokwi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Định nghĩa các chân kết nối (Theo diagram.json của bạn)
#define btnBLED 23 // Nút bấm xanh
#define pinBLED 21 // Đèn LED xanh
#define CLK 18     // TM1637 CLK
#define DIO 19     // TM1637 DIO
#define PIN_DHT 16 // Cảm biến DHT22

// Khởi tạo các đối tượng
TM1637Display display(CLK, DIO);
DHT dht(PIN_DHT, DHT22);
BlynkTimer timer;

// Biến toàn cục
ulong currentMiliseconds = 0;
bool systemON = false; // Trạng thái hệ thống (V0)
int countSeconds = 0;  // Bộ đếm thời gian (V3)

// Hàm kiểm tra thời gian không dùng delay
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (millis() - ulTimer < milisecond)
    return false;
  ulTimer = millis();
  return true;
}

// Hàm gửi dữ liệu cảm biến và đếm giây
void updateSystemData()
{
  if (systemON)
  {
    // 1. Đọc và gửi nhiệt độ (V1), độ ẩm (V2)
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h))
    {
      Blynk.virtualWrite(V1, t);
      Blynk.virtualWrite(V2, h);
    }

    // 2. Tăng bộ đếm và hiển thị
    countSeconds++;
    display.showNumberDec(countSeconds);
    Blynk.virtualWrite(V3, countSeconds); // Gửi lên ô "Đếm thời gian"
  }
}

// Xử lý nút bấm vật lý trên Wokwi (D23)
void checkPhysicalButton()
{
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50))
    return;

  int v = digitalRead(btnBLED);
  if (v == lastValue)
    return;
  lastValue = v;

  if (v == LOW)
  { // Khi nhấn nút
    systemON = !systemON;
    updateSystemState();
  }
}

// Hàm cập nhật trạng thái đèn và giao diện khi thay đổi ON/OFF
void updateSystemState()
{
  if (systemON)
  {
    digitalWrite(pinBLED, HIGH);
    Serial.println("Hệ thống: ON");
  }
  else
  {
    digitalWrite(pinBLED, LOW);
    display.clear();
    countSeconds = 0;
    Blynk.virtualWrite(V3, 0);
    Serial.println("Hệ thống: OFF");
  }
  // Đồng bộ nút gạt trên Blynk (V0)
  Blynk.virtualWrite(V0, systemON);
}

// Lắng nghe lệnh từ nút gạt Switch trên Blynk (V0)
BLYNK_WRITE(V0)
{
  systemON = param.asInt();
  updateSystemState();
}

void setup()
{
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(0x0f);

  Serial.print("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Thiết lập chu kỳ 1 giây để cập nhật dữ liệu
  timer.setInterval(1000L, updateSystemData);

  Serial.println("Ready!");
}

void loop()
{
  Blynk.run();
  timer.run();
  checkPhysicalButton();
}