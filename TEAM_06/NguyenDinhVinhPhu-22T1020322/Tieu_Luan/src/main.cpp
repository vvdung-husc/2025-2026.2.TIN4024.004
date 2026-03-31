#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ==========================================
// 1. CẤU HÌNH PHẦN CỨNG (HARDWARE CONFIG)
// ==========================================
#define PIN_DHT         16  // Chân Data của DHT22
#define DHT_TYPE        DHT22

#define PIN_RELAY       2   // Chân điều khiển tín hiệu Relay

#define PIN_OLED_SDA    13  // Chân I2C SDA cho OLED
#define PIN_OLED_SCL    12  // Chân I2C SCL cho OLED
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

// ==========================================
// 2. CẤU HÌNH THUẬT TOÁN (LOGIC CONFIG)
// ==========================================
// THUẬT TOÁN HYSTERESIS (TRỄ NHIỆT):
// Tránh hiện tượng Relay bật/tắt liên tục (Chattering) khi nhiệt độ dao động nhẹ quanh mức 30.
// Ví dụ: Lên 30°C -> Bật quạt. Phải hạ xuống 25°C -> Mới tắt quạt.
#define TEMP_ON         30.0 // Ngưỡng nhiệt độ CAO để BẬT quạt
#define TEMP_OFF        25 // Ngưỡng nhiệt độ THẤP để TẮT quạt

// ==========================================
// 3. KHỞI TẠO ĐỐI TƯỢNG (OBJECT INIT)
// ==========================================
DHT dht(PIN_DHT, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long lastReadTime = 0;
bool isFanOn = false; // Biến lưu trạng thái hiện tại của quạt

void setup() {
    Serial.begin(115200);

    // --- SETUP RELAY (FAIL-SAFE ĐẦU TIÊN) ---
    // Luôn set chân Relay là OUTPUT và kéo xuống LOW ngay khi khởi động
    // Đảm bảo quạt không tự nhiên rú lên khi ESP32 vừa cắm điện.
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);
    
    // Khởi động cảm biến
    dht.begin();

    // --- SETUP OLED CÓ BẪY LỖI ---
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("[CRITICAL ERROR] Không tìm thấy màn hình OLED!"));
        // Bẫy lỗi: Khóa chết chương trình ở đây nếu phần cứng màn hình lỗi
        // Tránh việc hệ thống chạy mù không kiểm soát được
        while (true); 
    }

    // Màn hình chào mừng
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(15, 20);
    display.println("SMART FAN SYSTEM");
    display.setCursor(35, 40);
    display.println("Loading...");
    display.display();
    delay(2000); // Dừng 2s cho DHT22 kịp khởi động (DHT22 cần thời gian warm-up)
}

void loop() {
    // Sử dụng Non-blocking timer (millis) thay cho delay()
    // Giúp CPU không bị treo, có thể làm việc khác (như nhận nút bấm, wifi...)
    if (millis() - lastReadTime >= 2000) {
        lastReadTime = millis();

        float t = dht.readTemperature();
        float h = dht.readHumidity();

        // ==========================================
        // KHỐI XỬ LÝ NGOẠI LỆ (EDGE CASE HANDLING)
        // ==========================================
        // Nếu dây cảm biến bị đứt hoặc nhiễu, hàm read() trả về NaN (Not a Number)
        if (isnan(t) || isnan(h)) {
            Serial.println("[ERROR] Mất kết nối với DHT22!");
            
            // FAIL-SAFE: Ngay lập tức ngắt Relay để đảm bảo an toàn điện
            digitalWrite(PIN_RELAY, LOW);
            isFanOn = false;

            // Cảnh báo lên màn hình
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(10, 20);
            display.println("SENSOR");
            display.setCursor(10, 40);
            display.println("ERROR!");
            display.display();
            
            return; // Thoát ngay khỏi block này, không chạy phần điều khiển bên dưới
        }

        // ==========================================
        // KHỐI LOGIC ĐIỀU KHIỂN (HYSTERESIS LOGIC)
        // ==========================================
        if (t >= TEMP_ON && !isFanOn) {
            // Nhiệt độ vượt ngưỡng CAO và quạt đang TẮT -> BẬT QUẠT
            digitalWrite(PIN_RELAY, HIGH);
            isFanOn = true;
            Serial.printf("Nhiệt độ: %.1f -> [BẬT QUẠT]\n", t);
        } 
        else if (t <= TEMP_OFF && isFanOn) {
            // Nhiệt độ thấp hơn ngưỡng THẤP và quạt đang BẬT -> TẮT QUẠT
            digitalWrite(PIN_RELAY, LOW);
            isFanOn = false;
            Serial.printf("Nhiệt độ: %.1f -> [TẮT QUẠT]\n", t);
        }
        // Trường hợp nhiệt độ nằm giữa TEMP_OFF và TEMP_ON (Ví dụ: 29.0):
        // Giữ nguyên trạng thái hiện tại (Đang bật thì bật tiếp, đang tắt thì tắt tiếp)

        // ==========================================
        // KHỐI HIỂN THỊ GIAO DIỆN OLED
        // ==========================================
        display.clearDisplay();
        
        // Vẽ viền trang trí
        display.drawRect(0, 0, 128, 64, WHITE);

        // Hiển thị trạng thái quạt
        display.setTextSize(1);
        display.setCursor(5, 5);
        display.print("QUAT: ");
        if (isFanOn) {
            display.println("DANG CHAY (ON)");
        } else {
            display.println("DUNG (OFF)");
        }
        
        display.drawLine(0, 18, 128, 18, WHITE);

        // Hiển thị Nhiệt độ
        display.setTextSize(2);
        display.setCursor(5, 25);
        display.print(t, 1);
        display.setTextSize(1);
        display.print("o"); // Ký hiệu độ
        display.setTextSize(2);
        display.print("C");

        // Hiển thị Độ ẩm ở góc phải
        display.setTextSize(1);
        display.setCursor(85, 25);
        display.print("AM DO");
        display.setCursor(85, 40);
        display.print(h, 0);
        display.print("%");

        display.display();
    }
}