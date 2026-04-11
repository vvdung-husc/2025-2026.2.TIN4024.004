// --- BLYNK ---
#define BLYNK_TEMPLATE_ID "TMPL6r03g4fU3"
#define BLYNK_TEMPLATE_NAME "BME280"
#define BLYNK_AUTH_TOKEN "Sfe6-iTQxcIoz_x4rGEjcLL5y8J4cUW3"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>   

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

// --- TELEGRAM ---
String botToken = "8608510632:AAFBVnBS9YC6_qVotbj7_AVFqlt8VgnR_HQ";   
String chatID   = "-1003292606653";     

// chống spam
unsigned long lastSend = 0;
int interval = 50000; // 50 giây

// Lưu trạng thái cảnh báo trước đó
String lastWarningType = "";

// --- OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- SENSOR ---
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

// --- WIFI ---
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Timer
BlynkTimer timer;

// ================= TELEGRAM =================
void sendTelegram(String message) {
  Serial.println("=== BAT DAU GUI TELEGRAM ===");
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi chua ket noi!");
    return;
  }
  
  Serial.println("WiFi da ket noi");
  
  HTTPClient http;
  
  // URL encode message - THỨ TỰ QUAN TRỌNG!
  // Encode % CUỐI CÙNG để không làm hỏng các mã %XX khác
  message.replace("&", "%26");
  message.replace("+", "%2B");
  message.replace("\n", "%0A");
  message.replace(" ", "%20");
  message.replace("!", "%21");
  message.replace("#", "%23");
  message.replace("$", "%24");
  message.replace("'", "%27");
  message.replace("(", "%28");
  message.replace(")", "%29");
  message.replace("*", "%2A");
  message.replace(",", "%2C");
  message.replace("/", "%2F");
  message.replace(":", "%3A");
  message.replace(";", "%3B");
  message.replace("=", "%3D");
  message.replace("?", "%3F");
  message.replace("@", "%40");
  message.replace("[", "%5B");
  message.replace("]", "%5D");
  
  String url = "https://api.telegram.org/bot" + botToken +
               "/sendMessage?chat_id=" + chatID +
               "&text=" + message;
  
  Serial.println("Dang gui...");
  
  http.begin(url);
  http.setTimeout(15000);
  
  int httpCode = http.GET();
  
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);
  
  if (httpCode > 0) {
    if (httpCode == 200) {
      Serial.println("GUI THANH CONG!");
    } else {
      String payload = http.getString();
      Serial.println("LOI: " + payload);
    }
  } else {
    Serial.print("LOI HTTP: ");
    Serial.println(http.errorToString(httpCode));
  }
  
  http.end();
  Serial.println("=== KET THUC GUI TELEGRAM ===\n");
}

// ================= SENSOR =================
void sendSensor() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  float pres = bmp.readPressure() / 100.0;

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Lỗi đọc DHT!");
    return;
  }

  // ===== OLED =====
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(" TRAM THOI TIET");
  display.println("----------------");
  display.print("Nhiet do: "); display.print(temp); display.println(" C");
  display.print("Do am   : "); display.print(hum); display.println(" %");
  display.print("Ap suat : "); display.print(pres); display.println(" hPa");
  display.display();

  // ===== BLYNK =====
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, hum);
  Blynk.virtualWrite(V2, pres);

  // ===== CẢNH BÁO =====
  String warningType = "";
  String warningMessage = "";

  // Kiểm tra nhiệt độ và độ ẩm
  if (temp < 10 && hum < 30) {
    warningType = "COLD_DRY";
    warningMessage = "Canh bao\n";
    warningMessage += "Nhiet do nho hon 10 do C - Nguy co ha than nhiet te cong giam mien dich\n";
    warningMessage += "Do am thap nho hon 30 phan tram - Tang nguy co benh ho hap";
  }
  else if (temp < 10 && hum > 80) {
    warningType = "COLD_HUMID";
    warningMessage = "Canh bao Cam giac lanh tang nguy co mac benh duong ho hap\n";
    warningMessage += "Da kho kich ung mat tang nguy co mac benh ve ho hap";
  }
  else if (temp > 30 && hum < 30) {
    warningType = "HOT_DRY";
    warningMessage = "Canh bao\n";
    warningMessage += "Nhiet do lon hon 30 do C - Nguy co soc nhiet say nang\n";
    warningMessage += "Do am thap nho hon 30 phan tram - Tang nguy co benh ho hap";
  }
  else if (temp > 30 && hum > 80) {
    warningType = "HOT_HUMID";
    warningMessage = "Canh bao Cam giac lanh tang nguy co mac benh duong ho hap\n";
    warningMessage += "Muc ly tuong it anh huong den suc khoe";
  }
  else if (temp < 10) {
    warningType = "COLD";
    warningMessage = "Canh bao\n";
    warningMessage += "Nhiet do nho hon 10 do C - Nguy co ha than nhiet te cong giam mien dich";
  }
  else if (temp > 30) {
    warningType = "HOT";
    warningMessage = "Canh bao\n";
    warningMessage += "Nhiet do lon hon 30 do C - Nguy co soc nhiet say nang";
  }
  else if (hum < 30) {
    warningType = "DRY";
    warningMessage = "Canh bao\n";
    warningMessage += "Do am thap nho hon 30 phan tram - Tang nguy co benh ho hap";
  }
  else if (hum > 80) {
    warningType = "HUMID";
    warningMessage = "Canh bao\n";
    warningMessage += "Do am cao lon hon 80 phan tram - Gay kho chiu tang nguy co nam da";
  }

  // ===== DEBUG CẢNH BÁO =====
  Serial.print("Temp: "); Serial.print(temp); Serial.print("C | ");
  Serial.print("Hum: "); Serial.print(hum); Serial.print("% | ");
  Serial.print("Pres: "); Serial.print(pres); Serial.println(" hPa");
  
  if (warningType != "") {
    Serial.println("CÓ CẢNH BÁO: " + warningType);
    Serial.println(warningMessage);
  } else {
    Serial.println("Không có cảnh báo");
  }

  // ===== GỬI TELEGRAM (CHỐNG SPAM) =====
  unsigned long currentTime = millis();
  unsigned long timeSinceLastSend = currentTime - lastSend;
  
  Serial.print("Thời gian từ lần gửi cuối: ");
  Serial.print(timeSinceLastSend / 1000);
  Serial.println(" giây");
  
  // Gửi nếu có cảnh báo MỚI hoặc đã qua 5 giây
  if (warningType != "" && (warningType != lastWarningType || timeSinceLastSend > interval)) {
    Serial.println(">>> SẼ GỬI TELEGRAM <<<");
    
    // Tạo message đầy đủ với dữ liệu cảm biến + cảnh báo
    String fullMessage = "CANH BAO MOI TRUONG\n\n";
    fullMessage += "Nhiet do " + String(temp, 1) + " do C\n";
    fullMessage += "Do am " + String(hum, 1) + " phan tram\n";
    fullMessage += "Ap suat " + String(pres, 1) + " hPa\n\n";
    fullMessage += warningMessage;
    
    sendTelegram(fullMessage);
    
    lastSend = currentTime;
    lastWarningType = warningType;
  } else if (warningType != "" && timeSinceLastSend <= interval) {
    Serial.print("⏳ Chờ thêm ");
    Serial.print((interval - timeSinceLastSend) / 1000);
    Serial.println(" giây nữa mới gửi");
  } else if (warningType == "") {
    // Reset trạng thái khi không còn cảnh báo
    lastWarningType = "";
  }

  Serial.println("----------------------------");
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Lỗi OLED");
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);

  dht.begin();

  if (!bmp.begin()) {
    Serial.println("Lỗi BMP180!");
    while (1);
  }

  // 🔥 KẾT NỐI WIFI TRƯỚC
  Serial.println("Đang kết nối WiFi...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // ⏳ ĐỢI WIFI ỔN ĐỊNH
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi đã kết nối!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // ✅ GỬI TELEGRAM TEST
    delay(2000);
    sendTelegram("ESP32 da ket noi thanh cong!");
  } else {
    Serial.println("\nKhông thể kết nối WiFi!");
  }

  timer.setInterval(2000L, sendSensor);
}

// ================= LOOP =================
void loop() {
  Blynk.run();
  timer.run();
}