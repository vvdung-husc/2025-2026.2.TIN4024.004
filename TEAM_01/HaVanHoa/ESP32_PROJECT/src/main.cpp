
// #include <Arduino.h>
// #include <DHT.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH110X.h>

// // Cấu hình chân cắm (Đã chuẩn hóa cho ESP8266)
// #define DHTPIN D3
// #define DHTTYPE DHT22
// #define LED_PIN D4   // LED tích hợp trên board NodeMCU thường là D4

// // Khởi tạo cảm biến và màn hình
// DHT dht(DHTPIN, DHTTYPE);
// Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

// void setup() {
//   Serial.begin(115200);
//   pinMode(LED_PIN, OUTPUT);
//   dht.begin();

//   // Khởi tạo I2C: D2 là SDA, D1 là SCL trên ESP8266
//   Wire.begin(D2, D1);

//   // Khởi tạo OLED
//   if (!display.begin(0x3C, true)) {
//     Serial.println(F("Khong tim thay man hinh OLED SH1106"));
//     for (;;); // Dừng chương trình nếu không có màn hình
//   }

//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(SH110X_WHITE);

//   // Màn hình khởi động
//   display.setCursor(20, 10);
//   display.println("KHOI DONG...");
//   display.setCursor(15, 30);
//   display.println("HVH_KK46");
//   display.setCursor(5, 48);
//   display.println("He thong giam sat");
//   display.display();
//   delay(2000);
// }

// void loop() {
//   // Đọc dữ liệu
//   float nhietDo = dht.readTemperature();
//   float doAm    = dht.readHumidity();

//   // Kiểm tra nếu cảm biến bị lỗi
//   if (isnan(nhietDo) || isnan(doAm)) {
//     Serial.println(F("Loi: Khong the doc tu cam bien DHT!"));

//     // Hiển thị lỗi lên OLED
//     display.clearDisplay();
//     display.setCursor(0, 10);
//     display.println("!!! LOI CAM BIEN !!!");
//     display.setCursor(0, 30);
//     display.println("Kiem tra DHT22");
//     display.setCursor(0, 50);
//     display.println("HA VAN HOA_K46");
//     display.display();
//     return;
//   }

//   // Xuất ra Serial Monitor (tiếng Việt không dấu cho Serial)
//   Serial.printf("Nhiet do: %.2f oC | Do am: %.2f %%\n", nhietDo, doAm);

//   // Hiển thị lên OLED
//   display.clearDisplay();

//   // --- Dòng tiêu đề ---
//   display.setTextSize(1);
//   display.setCursor(22, 0);
//   display.println("NCT_KK46");

//   // Kẻ đường ngang phân cách
//   display.drawLine(0, 10, 127, 10, SH110X_WHITE);

//   // --- Nhiệt độ ---
//   display.setCursor(0, 15);
//   display.println("NHIET DO:");
//   display.setTextSize(2);                // Chữ to để dễ đọc
//   display.setCursor(10, 25);
//   display.printf("%.1f C", nhietDo);

//   // Kẻ đường ngang giữa
//   display.setTextSize(1);
//   display.drawLine(0, 43, 127, 43, SH110X_WHITE);

//   // --- Độ ẩm ---
//   display.setCursor(0, 47);
//   display.println("DO AM:");
//   display.setTextSize(2);
//   display.setCursor(10, 52);
//   // Vì OLED 64px chiều cao, dùng size 1 cho dòng độ ẩm
//   display.setTextSize(1);
//   display.setCursor(45, 47);
//   display.printf("%.1f %%", doAm);

//   display.display();

//   // Blink LED báo hiệu chu kỳ hoạt động
//   digitalWrite(LED_PIN, LOW);   // ESP8266 LED sáng khi mức LOW
//   delay(500);
//   digitalWrite(LED_PIN, HIGH);
//   delay(1500); // Tổng 2 giây — DHT22 cần ít nhất 2s giữa các lần đọc
// }























/**
 * PROJECT SUMMARY: Hệ thống giám sát Môi trường - NCT_KK46
 * ---------------------------------------------------------
 * Board   : ESP8266 (NodeMCU v2/v3)
 * Cảm biến: DHT22 (D3) + PIR HC-SR501 (D5) + MQ2 Gas (A0)
 * Hiển thị: OLED SH1106 I2C (SDA->D2, SCL->D1)
 *
 * SƠ ĐỒ KẾT NỐI MQ2:
 *   VCC  -> 5V (hoặc 3.3V)
 *   GND  -> GND
 *   AOUT -> A0  (chân Analog duy nhất của ESP8266)
 *   DOUT -> Không dùng
 *
 * NGƯỠNG CẢNH BÁO MQ2 (có thể chỉnh tại mục CẤU HÌNH):
 *   - CANH_BAO_GAS_NHE  : 300  → Cảnh báo nhẹ (khói, hơi gas nhẹ)
 *   - CANH_BAO_GAS_NGUY : 600  → Nguy hiểm (gas đậm đặc)
 *   Giá trị ADC: 0-1023 (ESP8266 10-bit ADC)
 */

// #include <Arduino.h>
// #include <DHT.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH110X.h>

// // ============================================================
// // CẤU HÌNH CHÂN CẮM
// // ============================================================
// #define DHTPIN      D3
// #define DHTTYPE     DHT22
// #define LED_PIN     D4
// #define PIR_PIN     D5
// #define MQ2_PIN     A0    // Chân Analog duy nhất ESP8266

// // ============================================================
// // CẤU HÌNH NGƯỠNG & THỜI GIAN
// // ============================================================
// #define CANH_BAO_GAS_NHE      300   // ADC ~300 = bắt đầu có khói/gas
// #define CANH_BAO_GAS_NGUY     600   // ADC ~600 = nguy hiểm cao
// #define THOI_GIAN_DOC_SENSOR  2000  // Đọc DHT22 + MQ2 mỗi 2 giây
// #define THOI_GIAN_CANH_BAO    4000  // Giữ màn hình cảnh báo 4 giây
// #define LED_NHAP_NHAY_NHANH   150   // ms - LED nhấp nháy khi có gas
// #define LED_NHAP_NHAY_THUONG  1000  // ms - LED nháy bình thường

// // ============================================================
// // ENUM TRẠNG THÁI HỆ THỐNG
// // ============================================================
// enum TrangThaiManHinh {
//   MAN_HINH_CHINH,
//   CANH_BAO_CHUYEN_DONG,
//   CANH_BAO_GAS_NHE_MH,
//   CANH_BAO_GAS_NGUY_MH
// };

// // ============================================================
// // BIẾN TOÀN CỤC
// // ============================================================
// DHT dht(DHTPIN, DHTTYPE);
// Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

// float   nhietDo           = 0;
// float   doAm              = 0;
// int     giaTri_MQ2        = 0;    // Giá trị ADC thô 0-1023
// bool    coChuyenDong      = false;
// bool    gasNguyHiem       = false;
// bool    gasNhe            = false;

// TrangThaiManHinh trangThaiHienTai = MAN_HINH_CHINH;

// unsigned long thoiDiemDocSensor = 0;
// unsigned long thoiDiemCanhBao   = 0;
// unsigned long thoiDiemLed       = 0;
// bool          ledTrangThai       = false;

// // ============================================================
// // HÀM PHỤ TRỢ: Vẽ thanh mức gas
// // ============================================================
// void veThanhGas(int x, int y, int rong, int cao, int giaTriADC) {
//   display.drawRect(x, y, rong, cao, SH110X_WHITE);
//   int mucDo = map(constrain(giaTriADC, 0, 1023), 0, 1023, 0, rong - 2);
//   if (mucDo > 0) {
//     display.fillRect(x + 1, y + 1, mucDo, cao - 2, SH110X_WHITE);
//   }
// }

// // ============================================================
// // MÀN HÌNH CHÍNH
// // ============================================================
// void hienThiManHinhChinh() {
//   display.clearDisplay();
//   display.setTextColor(SH110X_WHITE);

//   // --- Tiêu đề ---
//   display.setTextSize(1);
//   display.setCursor(22, 0);
//   display.println("NCT_KK46");
//   display.drawLine(0, 9, 127, 9, SH110X_WHITE);

//   // --- Trạng thái PIR (góc phải) ---
//   display.setCursor(90, 0);
//   display.println(coChuyenDong ? "[CD]" : "[YEN]");

//   // --- Nhiệt độ ---
//   display.setCursor(0, 12);
//   display.println("NHIET DO:");
//   display.setTextSize(2);
//   display.setCursor(0, 21);
//   display.printf("%.1f", nhietDo);
//   display.setTextSize(1);
//   display.setCursor(78, 21);
//   display.println("oC");

//   // --- Đường kẻ ---
//   display.drawLine(0, 34, 127, 34, SH110X_WHITE);

//   // --- Độ ẩm ---
//   display.setTextSize(1);
//   display.setCursor(0, 37);
//   display.printf("DO AM: %.1f%%", doAm);

//   // --- Gas MQ2 ---
//   display.setCursor(0, 48);
//   display.printf("GAS:  %4d", giaTri_MQ2);

//   // Thanh mức gas trực quan
//   veThanhGas(60, 48, 65, 8, giaTri_MQ2);

//   // Nhãn mức gas
//   display.setCursor(0, 57);
//   if (gasNguyHiem) {
//     display.println(">> NGUY HIEM! <<");
//   } else if (gasNhe) {
//     display.println(">> Canh bao nhe");
//   } else {
//     display.println("   Binh thuong");
//   }

//   display.display();
// }

// // ============================================================
// // MÀN HÌNH CẢNH BÁO CHUYỂN ĐỘNG
// // ============================================================
// void hienThiCanhBaoCDong() {
//   display.clearDisplay();
//   display.drawRect(0, 0, 128, 64, SH110X_WHITE);
//   display.drawRect(2, 2, 124, 60, SH110X_WHITE);

//   display.setTextSize(1);
//   display.setCursor(4, 6);
//   display.println("!!! CANH BAO !!!");

//   display.setTextSize(1);
//   display.setCursor(10, 18);
//   display.println("PHAT HIEN");
//   display.setCursor(6, 28);
//   display.println("CHUYEN DONG!");

//   display.drawLine(5, 40, 122, 40, SH110X_WHITE);

//   display.setCursor(6, 43);
//   display.printf("Nhiet do: %.1f oC", nhietDo);
//   display.setCursor(6, 53);
//   display.printf("Do am:    %.1f %%", doAm);

//   display.display();
// }

// // ============================================================
// // MÀN HÌNH CẢNH BÁO GAS NHẸ
// // ============================================================
// void hienThiCanhBaoGasNhe() {
//   display.clearDisplay();
//   display.drawRect(0, 0, 128, 64, SH110X_WHITE);

//   display.setTextSize(1);
//   display.setCursor(15, 4);
//   display.println(">> CANH BAO <<");

//   display.setCursor(8, 16);
//   display.println("PHAT HIEN KHI GAS");
//   display.setCursor(22, 26);
//   display.println("MUC DO NHE");

//   display.drawLine(5, 37, 122, 37, SH110X_WHITE);

//   display.setCursor(6, 40);
//   display.printf("Gia tri: %d / 1023", giaTri_MQ2);

//   // Thanh mức
//   veThanhGas(6, 50, 116, 10, giaTri_MQ2);

//   display.setCursor(6, 55);
//   // Hiện % mức nguy hiểm
//   int phanTram = map(constrain(giaTri_MQ2, 0, 1023), 0, 1023, 0, 100);
//   display.printf("Muc: %d%%", phanTram);

//   display.display();
// }

// // ============================================================
// // MÀN HÌNH CẢNH BÁO GAS NGUY HIỂM
// // ============================================================
// void hienThiCanhBaoGasNguy() {
//   display.clearDisplay();

//   // Viền đôi nhấn mạnh
//   display.drawRect(0, 0, 128, 64, SH110X_WHITE);
//   display.drawRect(3, 3, 122, 58, SH110X_WHITE);

//   display.setTextSize(1);
//   display.setCursor(12, 7);
//   display.println("!!! NGUY HIEM !!!");

//   display.setTextSize(2);
//   display.setCursor(14, 20);
//   display.println("GAS CAO!");

//   display.setTextSize(1);
//   display.drawLine(5, 38, 122, 38, SH110X_WHITE);

//   display.setCursor(6, 41);
//   display.printf("ADC : %d", giaTri_MQ2);

//   display.setCursor(6, 51);
//   display.println("KIEM TRA NGAY!");

//   display.display();
// }

// // ============================================================
// // XỬ LÝ LED
// // ============================================================
// void xuLyLed(unsigned long thoiDiemHienTai) {
//   int tocDoNhay = (gasNguyHiem || gasNhe || coChuyenDong)
//                   ? LED_NHAP_NHAY_NHANH
//                   : LED_NHAP_NHAY_THUONG;

//   if (thoiDiemHienTai - thoiDiemLed >= (unsigned long)tocDoNhay) {
//     thoiDiemLed  = thoiDiemHienTai;
//     ledTrangThai = !ledTrangThai;
//     // ESP8266: LOW = sáng, HIGH = tắt
//     digitalWrite(LED_PIN, ledTrangThai ? LOW : HIGH);
//   }
// }

// // ============================================================
// // SETUP
// // ============================================================
// void setup() {
//   Serial.begin(115200);
//   Serial.println(F("\n=== NCT_KK46 - HE THONG GIAM SAT MOI TRUONG ==="));

//   pinMode(LED_PIN, OUTPUT);
//   pinMode(PIR_PIN, INPUT);
//   digitalWrite(LED_PIN, HIGH); // Tắt LED khi khởi động

//   dht.begin();
//   Wire.begin(D2, D1);

//   if (!display.begin(0x3C, true)) {
//     Serial.println(F("Khong tim thay OLED SH1106!"));
//     for (;;);
//   }

//   display.clearDisplay();
//   display.setTextColor(SH110X_WHITE);

//   // --- Màn hình khởi động ---
//   display.setTextSize(1);
//   display.setCursor(10, 2);
//   display.println("HE THONG GIAM SAT");
//   display.setTextSize(2);
//   display.setCursor(16, 16);
//   display.println("NCT_KK46");
//   display.setTextSize(1);
//   display.drawLine(0, 34, 127, 34, SH110X_WHITE);
//   display.setCursor(5, 38);
//   display.println("DHT22 | PIR | MQ2");
//   display.setCursor(18, 50);
//   display.println("Dang khoi dong...");
//   display.display();

//   Serial.println(F("Cho PIR on dinh... (5 giay)"));
//   delay(5000);
//   Serial.println(F("San sang hoat dong!"));
// }

// // ============================================================
// // LOOP CHÍNH
// // ============================================================
// void loop() {
//   unsigned long thoiDiemHienTai = millis();

//   // --- 1. Đọc PIR liên tục ---
//   coChuyenDong = digitalRead(PIR_PIN);

//   // --- 2. Đọc DHT22 + MQ2 mỗi 2 giây ---
//   if (thoiDiemHienTai - thoiDiemDocSensor >= THOI_GIAN_DOC_SENSOR) {
//     thoiDiemDocSensor = thoiDiemHienTai;

//     // Đọc DHT22
//     float t = dht.readTemperature();
//     float h = dht.readHumidity();
//     if (!isnan(t) && !isnan(h)) {
//       nhietDo = t;
//       doAm    = h;
//     }

//     // Đọc MQ2 (lấy trung bình 5 mẫu để ổn định hơn)
//     long tong = 0;
//     for (int i = 0; i < 5; i++) {
//       tong += analogRead(MQ2_PIN);
//       delay(10);
//     }
//     giaTri_MQ2  = tong / 5;
//     gasNguyHiem = (giaTri_MQ2 >= CANH_BAO_GAS_NGUY);
//     gasNhe      = (giaTri_MQ2 >= CANH_BAO_GAS_NHE && giaTri_MQ2 < CANH_BAO_GAS_NGUY);

//     Serial.printf("[NCT_KK46] NhietDo: %.1f oC | DoAm: %.1f%% | MQ2: %d | PIR: %s | Gas: %s\n",
//       nhietDo, doAm, giaTri_MQ2,
//       coChuyenDong ? "CO" : "KHONG",
//       gasNguyHiem  ? "NGUY HIEM" : (gasNhe ? "NHE" : "BINH THUONG")
//     );
//   }

//   // --- 3. Xác định màn hình ưu tiên (gas nguy > gas nhẹ > chuyển động > chính) ---
//   if (gasNguyHiem) {
//     trangThaiHienTai  = CANH_BAO_GAS_NGUY_MH;
//     thoiDiemCanhBao   = thoiDiemHienTai;
//   } else if (gasNhe) {
//     trangThaiHienTai  = CANH_BAO_GAS_NHE_MH;
//     thoiDiemCanhBao   = thoiDiemHienTai;
//   } else if (coChuyenDong) {
//     trangThaiHienTai  = CANH_BAO_CHUYEN_DONG;
//     thoiDiemCanhBao   = thoiDiemHienTai;
//   } else if (thoiDiemHienTai - thoiDiemCanhBao >= THOI_GIAN_CANH_BAO) {
//     trangThaiHienTai  = MAN_HINH_CHINH;
//   }

//   // --- 4. Hiển thị theo trạng thái ---
//   switch (trangThaiHienTai) {
//     case CANH_BAO_GAS_NGUY_MH:   hienThiCanhBaoGasNguy();  break;
//     case CANH_BAO_GAS_NHE_MH:    hienThiCanhBaoGasNhe();   break;
//     case CANH_BAO_CHUYEN_DONG:   hienThiCanhBaoCDong();    break;
//     default:                     hienThiManHinhChinh();    break;
//   }

//   // --- 5. Xử lý LED ---
//   xuLyLed(thoiDiemHienTai);

//   delay(100);
// }











/**
 * ============================================================================
 * DỰ ÁN: HỆ THỐNG GIÁM SÁT ĐA CẢM BIẾN (NCT_KK46)
 * ============================================================================
 * Phần cứng:
 *  - ESP8266 NodeMCU v2/v3
 *  - OLED SH1106 128x64 (I2C: SDA=D2, SCL=D1)
 *  - DHT22 (Data=D3)
 *  - PIR HC-SR501 (Out=D5)
 *  - MQ-2 Gas (Aout=A0)
 *  - LED tích hợp (D4)
 * 
 * Cập nhật mới nhất:
 *  - Xử lý PIR không làm chiếm dụng màn hình quá lâu (Cooldown 15s).
 *  - Hiển thị cảnh báo Gas ưu tiên cao nhất.
 *  - Hiển thị thông số môi trường liên tục.
 * ============================================================================
 */

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ============================================================================
// 1. CẤU HÌNH CHÂN CẮM (PIN CONFIG)
// ============================================================================
#define DHT_PIN       D3      // Chân dữ liệu DHT22
#define DHT_TYPE      DHT22   // Loại cảm biến
#define LED_PIN       D4      // LED tích hợp (LOW = Sáng)
#define PIR_PIN       D5      // Chân tín hiệu PIR
#define MQ2_PIN       A0      // Chân Analog đọc khí gas

// ============================================================================
// 2. CẤU HÌNH NGƯỠNG & THỜI GIAN (CONFIG)
// ============================================================================
// Ngưỡng khí gas (0-1023). Cần hiệu chỉnh theo thực tế môi trường.
// Giá trị tham khảo: Không khí sạch ~100-300. Có khói/gas ~400+
#define GAS_THRESHOLD     400   

// Thời gian hiển thị cảnh báo (ms)
#define PIR_ALERT_TIME    2000   // Chỉ hiện màn hình cảnh báo PIR trong 2 giây
#define PIR_COOLDOWN_TIME 15000  // Sau đó nghỉ 15 giây không báo nữa (tránh spam)
#define GAS_ALERT_TIME    8000   // Cảnh báo gas trong 8 giây

// Thời gian đọc cảm biến
#define DHT_INTERVAL      2000   // DHT22 cần ít nhất 2s giữa các lần đọc

// ============================================================================
// 3. KHỞI TẠO ĐỐI TƯỢNG (OBJECTS)
// ============================================================================
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

// ============================================================================
// 4. BIẾN TOÀN CỤC (GLOBAL VARIABLES)
// ============================================================================
float temperature = 0.0;
float humidity    = 0.0;
int   gasValue    = 0;

// Trạng thái PIR
bool pirTriggered   = false;   // Đang có chuyển động mới
bool pirInCooldown  = false;   // Đang trong thời gian nghỉ
unsigned long pirAlertStart   = 0;
unsigned long pirCooldownStart = 0;

// Trạng thái GAS
bool gasAlert       = false;
unsigned long gasAlertStart   = 0;

// Thời gian đọc DHT
unsigned long lastDHTRead = 0;

// ============================================================================
// 5. CÁC HÀM HỖ TRỢ (HELPER FUNCTIONS)
// ============================================================================

// --- Đọc khí gas ổn định (Lấy trung bình 5 mẫu) ---
int readGasSensor() {
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(MQ2_PIN);
    delay(10);
  }
  return sum / 5;
}

// --- Hiển thị màn hình chính (Nhiệt độ, Độ ẩm, Gas, Icon PIR) ---
void displayMainScreen() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  
  // 1. Tiêu đề
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.println("HA VAN HOA_K46");
  
  // 2. Icon trạng thái PIR ở góc phải (Nhấp nháy nếu đang cooldown)
  if (pirInCooldown) {
    // Đang trong thời gian nghỉ (vẫn theo dõi nhưng không báo động)
    if ((millis() / 500) % 2 == 0) {
      display.fillRect(118, 0, 10, 10, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    } else {
      display.drawRect(118, 0, 10, 10, SH110X_WHITE);
      display.setTextColor(SH110X_WHITE);
    }
    display.setCursor(120, 1);
    display.print("P");
  } else {
    // Bình thường
    display.drawRect(118, 0, 10, 10, SH110X_WHITE);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(120, 1);
    display.print("P");
  }
  display.setTextColor(SH110X_WHITE); // Reset màu
  
  display.drawLine(0, 10, 127, 10, SH110X_WHITE);
  
  // 3. Hàng thông số 1 (Nhiệt & Ẩm)
  display.setCursor(0, 14);
  display.print("T:");
  display.setTextSize(1);
  display.printf("%.1fC", temperature);
  
  display.setCursor(55, 14);
  display.print("H:");
  display.printf("%.1f%%", humidity);
  
  // 4. Hàng thông số 2 (Khí Gas & Thanh tiến trình)
  display.setCursor(0, 28);
  display.print("G:");
  display.print(gasValue);
  
  // Vẽ thanh tiến trình gas
  int barWidth = map(constrain(gasValue, 0, 1023), 0, 1023, 0, 80);
  display.drawRect(25, 30, 80, 5, SH110X_WHITE);
  if (barWidth > 0) {
    // Nếu gas cao thì tô đậm hơn
    if (gasValue > GAS_THRESHOLD) {
      display.fillRect(26, 31, barWidth - 1, 3, SH110X_WHITE);
    } else {
      // Kẻ vạch chéo cho mức bình thường
      for (int i = 26; i < 26 + barWidth; i += 4) {
        display.drawLine(i, 31, i + 2, 33, SH110X_WHITE);
      }
    }
  }
  
  // 5. Đường phân cách dưới
  display.drawLine(0, 40, 127, 40, SH110X_WHITE);
  
  // 6. Trạng thái hệ thống
  display.setCursor(0, 45);
  display.print("PIR:");
  display.print(pirInCooldown ? "WAIT" : "READY");
  
  display.setCursor(60, 45);
  display.print("GAS:");
  display.print(gasAlert ? "ALERT" : "OK");
  
  // 7. Footer
  display.drawLine(0, 55, 127, 55, SH110X_WHITE);
  display.setCursor(30, 58);
  display.setTextSize(1);
  display.print("Monitoring System");
  
  display.display();
}

// --- Hiển thị cảnh báo PIR (Toàn màn hình trong thời gian ngắn) ---
void displayPIRAlert() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  
  display.setTextSize(1);
  display.setCursor(25, 0);
  display.println("HA VAN HOA_K46");
  display.drawLine(0, 10, 127, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(15, 25);
  display.println("MOTION!");
  display.setCursor(35, 45);
  display.println("DETECTED");
  
  // Vẽ icon người chạy đơn giản
  display.drawLine(110, 20, 115, 30, SH110X_WHITE);
  display.drawLine(115, 30, 108, 38, SH110X_WHITE);
  display.drawLine(115, 30, 122, 38, SH110X_WHITE);
  display.drawLine(110, 20, 120, 20, SH110X_WHITE);
  
  display.display();
}

// --- Hiển thị cảnh báo GAS (Ưu tiên cao nhất) ---
void displayGasAlert() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  
  display.setTextSize(1);
  display.setCursor(25, 0);
  display.println("NCT_KK46");
  display.drawLine(0, 10, 127, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println("CANH BAO");
  display.setCursor(25, 40);
  display.println("KHI GAS!");
  
  display.setTextSize(1);
  display.setCursor(10, 55);
  display.print("Level: ");
  display.print(gasValue);
  
  // Vẽ icon lửa
  display.drawLine(115, 25, 120, 35, SH110X_WHITE);
  display.drawLine(120, 35, 125, 25, SH110X_WHITE);
  display.drawLine(115, 25, 125, 25, SH110X_WHITE);
  
  display.display();
}

// ============================================================================
// 6. SETUP & LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== KHOI DONG HE THONG ===");
  
  // Cấu hình chân
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  digitalWrite(LED_PIN, HIGH); // LED tắt (HIGH) khi bắt đầu
  
  // Khởi tạo cảm biến
  dht.begin();
  
  // Khởi tạo I2C & OLED
  Wire.begin(D2, D1); // SDA, SCL
  if (!display.begin(0x3C, true)) {
    Serial.println("Loi: Khong tim thay OLED!");
    while (1) {
      digitalWrite(LED_PIN, LOW); delay(500); digitalWrite(LED_PIN, HIGH); delay(500);
    }
  }
  
  // Màn hình chào
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(15, 10); display.println("DA KHOI DONG");
  display.setCursor(25, 25); display.println("NCT_KK46");
  display.setCursor(10, 40); display.println("Multi-Sensor");
  display.setCursor(20, 54); display.println("Monitoring");
  display.display();
  
  delay(2000);
  
  Serial.println("He thong san sang. MQ-2 dang preheat...");
}

void loop() {
  unsigned long now = millis();
  
  // ==========================================================================
  // 1. XỬ LÝ PIR (CHỐNG NHẢY & COOLDOWN)
  // ==========================================================================
  int pirState = digitalRead(PIR_PIN);
  
  // Nếu có tín hiệu HIGH và không đang trong thời gian nghỉ (cooldown)
  if (pirState == HIGH && !pirInCooldown && !pirTriggered) {
    pirTriggered = true;
    pirAlertStart = now;
    pirInCooldown = false;
    Serial.println(">>> [PIR] Phat hien chuyen dong!");
    
    // Nhấp nháy LED báo động nhanh
    for(int i=0; i<3; i++) {
      digitalWrite(LED_PIN, LOW); delay(50);
      digitalWrite(LED_PIN, HIGH); delay(50);
    }
  }
  
  // Kiểm tra thời gian hiển thị cảnh báo PIR (2 giây)
  if (pirTriggered) {
    if (now - pirAlertStart >= PIR_ALERT_TIME) {
      pirTriggered = false;       // Tắt trạng thái báo động full màn hình
      pirInCooldown = true;       // Bật chế độ nghỉ
      pirCooldownStart = now;     // Bắt đầu đếm thời gian nghỉ
      Serial.println("<<< [PIR] Het thoi gian bao, vao che do nghi.");
    }
  }
  
  // Kiểm tra thời gian nghỉ (Cooldown 15 giây)
  if (pirInCooldown) {
    if (now - pirCooldownStart >= PIR_COOLDOWN_TIME) {
      pirInCooldown = false;      // Hết nghỉ, sẵn sàng phát hiện lại
      Serial.println("<<< [PIR] Het thoi gian nghi, san sang.");
    }
  }
  
  // ==========================================================================
  // 2. XỬ LÝ GAS (MQ-2)
  // ==========================================================================
  gasValue = readGasSensor();
  
  if (gasValue >= GAS_THRESHOLD && !gasAlert) {
    gasAlert = true;
    gasAlertStart = now;
    Serial.printf(">>> [GAS] Canh bao! Level: %d\n", gasValue);
  }
  
  // Tự tắt cảnh báo gas sau thời gian quy định nếu mức khí giảm
  if (gasAlert) {
    if (now - gasAlertStart >= GAS_ALERT_TIME) {
      if (gasValue < (GAS_THRESHOLD * 0.8)) { // Chỉ tắt nếu khí đã giảm xuống dưới 80% ngưỡng
        gasAlert = false;
        Serial.println("<<< [GAS] Da an toan.");
      } else {
        gasAlertStart = now; // Reset thời gian nếu khí vẫn cao
      }
    }
  }
  
  // ==========================================================================
  // 3. ĐỌC DHT22 (2 GIÂY/LẦN)
  // ==========================================================================
  if (now - lastDHTRead >= DHT_INTERVAL) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    
    if (!isnan(temp) && !isnan(hum)) {
      temperature = temp;
      humidity = hum;
      // Serial.printf("[DHT] T: %.1f | H: %.1f\n", temp, hum);
    }
    lastDHTRead = now;
  }
  
  // ==========================================================================
  // 4. HIỂN THỊ OLED (ƯU TIÊN: GAS > PIR > MAIN)
  // ==========================================================================
  
  if (gasAlert) {
    // Ưu tiên 1: Cảnh báo Gas (Nguy hiểm nhất)
    displayGasAlert();
    digitalWrite(LED_PIN, LOW); // LED sáng liên tục
    delay(100); 
    return; // Thoát vòng loop để ưu tiên xử lý alert
  }
  
  if (pirTriggered) {
    // Ưu tiên 2: Cảnh báo PIR (Chỉ 2 giây)
    displayPIRAlert();
    digitalWrite(LED_PIN, LOW);
    delay(100);
    return;
  }
  
  // Bình thường: Hiển thị màn hình chính
  displayMainScreen();
  
  // LED heartbeat (Nhấp nháy chậm báo hiệu hệ thống sống)
  static bool ledState = false;
  if (now % 1000 < 500) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? LOW : HIGH);
  }
  
  delay(200); // Delay nhỏ để ổn định vòng lặp
}