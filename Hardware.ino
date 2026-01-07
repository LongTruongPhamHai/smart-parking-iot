#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PCF8574.h>
#include <ESP32Servo.h>
// ===== THƯ VIỆN WEB =====
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
const char *apiUrl = "https://your-backend.com/api/fire-alert"; // URL API của bạn

// ===== CẤU HÌNH WIFI (SỬA Ở ĐÂY) =====
const char *ssid = "DUC CO";        // <--- Điền Tên WiFi
const char *password = "ducco2711"; // <--- Điền Mật khẩu

WebServer server(80); // Khởi tạo Web Server

// ================= LCD + PCF =================
#define LCD_ADDR 0x27
#define PCF_ADDR 0x20
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
PCF8574 pcf(PCF_ADDR);

// ================= CỔNG =====================
#define TRIG_IN 33
#define ECHO_IN 32
#define SERVO_IN 25

#define TRIG_OUT 27
#define ECHO_OUT 14
#define SERVO_OUT 13

// ================= CHỖ ĐỖ ===================
#define TRIG_P1 4
#define ECHO_P1 16
#define TRIG_P2 5
#define ECHO_P2 17
#define TRIG_P3 18
#define ECHO_P3 19

// ================= CẢM BIẾN =================
#define FIRE_PIN 26
#define GAS_PIN 34

// ================= CẢM BIẾN MƯA & MÁI CHE ====
#define RAIN_PIN 35       // Chân cảm biến mưa
#define SERVO_ROOF_PIN 23 // Chân Servo mái che

// ================= ÁNH SÁNG (MỚI) ===========
#define LIGHT_SENSOR_PIN 36 // Cảm biến ánh sáng
#define LED_PIN 2           // Đèn LED chiếu sáng
#define BUZZER_PIN 12       // Còi Báo Động

// ================= THAM SỐ ==================
#define MAX_CARS 3
#define DIST_THRESHOLD 10
#define SLOT_THRESHOLD 15
#define PASSWORD_TIMEOUT 5000
#define GATE_CLOSE_DELAY 4000
#define PRICE_PER_HOUR 20000
#define SLOT_UPDATE_INTERVAL 500

Servo servoIn, servoOut;
Servo servoRoof;

// ================= KEYPAD ===================
char keymap[4][4] = {
    {'D', 'C', 'B', 'A'},
    {'#', '9', '6', '3'},
    {'0', '8', '5', '2'},
    {'*', '7', '4', '1'}};

// ================= DỮ LIỆU XE ===============
String carID[MAX_CARS];
unsigned long carTimeIn[MAX_CARS];

// Biến lưu lịch sử cho Web
// ================= SLOT & LOG (MỚI) =========
bool slot[3];
bool lastSlot[3];
unsigned long lastSlotUpdate = 0;

// THAY ĐỔI: Tách thành 3 mảng riêng
String logCars[10];  // Lưu 10 xe gần nhất
String logSystem[5]; // Lưu 5 hoạt động hệ thống (Barie, Mái che)
String logAlarm[5];  // Lưu 5 cảnh báo nguy hiểm

// ================= TRẠNG THÁI ===============
bool waitingInput = false;
bool gateIsIn = true;
bool gateOpen = false;
String inputID = "";
unsigned long inputTimer = 0;
unsigned long gateTimer = 0;
// ================= LOGIC MƯA (MỚI) ==========
bool isRaining = false;
unsigned long rainStopTime = 0;
const unsigned long RAIN_DELAY = 5000;
bool isDark = false; // Biến lưu trạng thái trời tối

// ================= LỬA + GAS ================
bool fireMode = false;
unsigned long fireStart = 0;
unsigned long fireClear = 0;
bool gasMode = false;
unsigned long gasStart = 0;
unsigned long gasClear = 0;
// Biến tạo nhịp tít tít cho còi (MỚI)
unsigned long beepTimer = 0;
bool beepState = false;
// ================= HÀM WEB SERVER ===========
// ================= HÀM LOGGING (MỚI) ========
// Hàm gốc để thêm dữ liệu vào mảng
void addLog(String array[], int size, String msg)
{
  for (int i = size - 1; i > 0; i--)
    array[i] = array[i - 1];
  array[0] = msg;
}

// Các hàm gọi tắt cho gọn
void logCar(String msg) { addLog(logCars, 10, msg); }
void logSys(String msg) { addLog(logSystem, 5, msg); }
void logDanger(String msg) { addLog(logAlarm, 5, msg); }

String getWebPage()
{
  String ptr = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<title>Quan Ly Bai Xe</title>";

  // CSS MỚI: Thêm màu sắc cho từng bảng
  ptr += "<style>";
  ptr += "body{font-family:Arial;margin:10px;text-align:center}";
  ptr += ".box{border:1px solid #ddd;padding:10px;margin-bottom:15px;border-radius:5px}";
  ptr += ".slot{display:inline-block;width:30%;height:50px;line-height:50px;color:white;font-weight:bold;margin:1%;border-radius:5px}";
  ptr += ".full{background:#e74c3c}.empty{background:#2ecc71}";
  ptr += "table{width:100%;border-collapse:collapse;margin-top:5px}";
  ptr += "th,td{border:1px solid #ddd;padding:6px;text-align:left;font-size:13px}";
  ptr += "th{background:#f4f4f4}";
  ptr += ".h-car th{background:#d6eaf8; color:#154360}";    // Xanh dương (Xe)
  ptr += ".h-sys th{background:#d5f5e3; color:#186a3b}";    // Xanh lá (Hệ thống)
  ptr += ".h-danger th{background:#fadbd8; color:#78281f}"; // Đỏ (Nguy hiểm)
  ptr += "</style>";
  ptr += "<script>setInterval(function(){location.reload();}, 3000);</script></head><body>";

  ptr += "<h2>HE THONG BAI XE</h2>";

  // 1. Trạng thái Slots & Mái che
  ptr += "<div class='box'><h3>TRANG THAI</h3>";
  ptr += "<p>Mai che: <b>" + String(isRaining ? "DANG DONG (Mua)" : "DANG MO (Nang)") + "</b></p>";
  ptr += "Den san: <b>" + String(isDark ? "DANG BAT (Toi)" : "DANG TAT (Sang)") + "</b></p>"; // Hiện trạng thái đèn
  for (int i = 0; i < 3; i++)
  {
    if (slot[i])
      ptr += "<div class='slot full'>XE " + String(i + 1) + "</div>";
    else
      ptr += "<div class='slot empty'>TRONG</div>";
  }
  ptr += "</div>";

  // 2. Bảng Lịch sử Xe
  ptr += "<div class='box'><h3 style='color:#154360'>LICH SU XE RA/VAO</h3><table class='h-car'>";
  for (int i = 0; i < 10; i++)
    if (logCars[i] != "")
      ptr += "<tr><td>" + logCars[i] + "</td></tr>";
  ptr += "</table></div>";

  // 3. Bảng Hoạt động Hệ thống
  ptr += "<div class='box'><h3 style='color:#186a3b'>HOAT DONG HE THONG</h3><table class='h-sys'>";
  for (int i = 0; i < 5; i++)
    if (logSystem[i] != "")
      ptr += "<tr><td>" + logSystem[i] + "</td></tr>";
  ptr += "</table></div>";

  // 4. Bảng Cảnh báo
  ptr += "<div class='box'><h3 style='color:#c0392b'>CANH BAO NGUY HIEM</h3><table class='h-danger'>";
  for (int i = 0; i < 5; i++)
    if (logAlarm[i] != "")
      ptr += "<tr><td style='color:red;font-weight:bold'>" + logAlarm[i] + "</td></tr>";
  ptr += "</table></div>";

  ptr += "</body></html>";
  return ptr;
}

void handleRoot()
{
  server.send(200, "text/html", getWebPage());
}

// ================= HÀM LOGIC CŨ =============
long getDistance(int t, int e)
{
  digitalWrite(t, LOW);
  delayMicroseconds(2);
  digitalWrite(t, HIGH);
  delayMicroseconds(10);
  digitalWrite(t, LOW);
  long d = pulseIn(e, HIGH, 25000);
  if (d == 0)
    return 999;
  return d * 0.034 / 2;
}

void updateSlots()
{
  slot[0] = getDistance(TRIG_P1, ECHO_P1) < SLOT_THRESHOLD;
  slot[1] = getDistance(TRIG_P2, ECHO_P2) < SLOT_THRESHOLD;
  slot[2] = getDistance(TRIG_P3, ECHO_P3) < SLOT_THRESHOLD;
}

void drawSlots()
{
  lcd.setCursor(0, 0);
  lcd.print("CHO DO: ");
  for (int i = 0; i < 3; i++)
    lcd.print(slot[i] ? "[x]" : "[ ]");
}

void showHome()
{
  lcd.clear();
  drawSlots();
  lcd.setCursor(0, 1);
  // HIỂN THỊ IP ĐỂ BIẾT ĐƯỜNG VÀO WEB
  if (WiFi.status() == WL_CONNECTED)
  {
    lcd.print("IP:");
    lcd.print(WiFi.localIP());
  }
  else
  {
    lcd.print("Dang cho xe...");
  }
  waitingInput = false;
  inputID = "";
}

int countCars()
{
  int c = 0;
  for (int i = 0; i < MAX_CARS; i++)
    if (carID[i] != "")
      c++;
  return c;
}

char scanKeypad()
{
  for (int r = 0; r < 4; r++)
  {
    pcf.write8(0xFF);
    pcf.write(r, LOW);
    for (int c = 0; c < 4; c++)
    {
      if (pcf.read(c + 4) == LOW)
      {
        while (pcf.read(c + 4) == LOW)
          ;
        return keymap[r][c];
      }
    }
  }
  return 0;
}

int findCar(String id)
{
  for (int i = 0; i < MAX_CARS; i++)
    if (carID[i] == id)
      return i;
  return -1;
}

bool addCar(String id)
{
  if (findCar(id) != -1)
    return false;
  for (int i = 0; i < MAX_CARS; i++)
  {
    if (carID[i] == "")
    {
      carID[i] = id;
      carTimeIn[i] = millis();
      logCar("VAO: TK " + id);
      return true;
    }
  }
  return false;
}

// ================= SETUP ====================
void setup()
{
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pcf.begin();

  // KẾT NỐI WIFI
  lcd.print("Ket noi WiFi...");
  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20)
  {
    delay(500);
    retry++;
  }

  // KHỞI CHẠY SERVER
  server.on("/", handleRoot);
  server.begin();

  pinMode(TRIG_IN, OUTPUT);
  pinMode(ECHO_IN, INPUT);
  pinMode(TRIG_OUT, OUTPUT);
  pinMode(ECHO_OUT, INPUT);
  pinMode(TRIG_P1, OUTPUT);
  pinMode(ECHO_P1, INPUT);
  pinMode(TRIG_P2, OUTPUT);
  pinMode(ECHO_P2, INPUT);
  pinMode(TRIG_P3, OUTPUT);
  pinMode(ECHO_P3, INPUT);

  pinMode(FIRE_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT); // Cấu hình chân cảm biến mưa
  // SETUP ÁNH SÁNG & LED
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Mặc định tắt đèn
  servoIn.attach(SERVO_IN);
  servoOut.attach(SERVO_OUT);
  servoRoof.attach(SERVO_ROOF_PIN); // Gắn servo mái che
  servoIn.write(0);
  servoOut.write(0);
  servoRoof.write(0); // Mặc định mở mái che (0 độ)
  updateSlots();
  memcpy(lastSlot, slot, sizeof(slot));
  showHome();
}

// ================= LOOP =====================
void loop()
{
  server.handleClient(); // <--- BẮT BUỘC ĐỂ WEB CHẠY

  // ===== UPDATE SLOT REALTIME =====
  if (millis() - lastSlotUpdate > SLOT_UPDATE_INTERVAL)
  {
    lastSlotUpdate = millis();
    updateSlots();
    if (memcmp(slot, lastSlot, sizeof(slot)) != 0)
    {
      memcpy(lastSlot, slot, sizeof(slot));
      if (!waitingInput && !gateOpen)
        drawSlots();
    }
  }
  // ===== ÁNH SÁNG (TỰ ĐỘNG BẬT ĐÈN) =====
  int lightVal = digitalRead(LIGHT_SENSOR_PIN);
  // Lưu ý: Tùy loại cảm biến, có thể HIGH là Tối hoặc HIGH là Sáng.
  // Thông thường: HIGH = Tối (khi che tay vào)
  if (lightVal == HIGH && !isDark)
  {
    isDark = true;
    digitalWrite(LED_PIN, HIGH); // Bật đèn
    logSys("TOI: Da bat den LED");
  }
  else if (lightVal == LOW && isDark)
  {
    isDark = false;
    digitalWrite(LED_PIN, LOW); // Tắt đèn
    logSys("SANG: Da tat den LED");
  }
  // ===== XỬ LÝ MƯA (CÓ TRỄ 5 GIÂY) =========================
  // ========================================================
  int rainStatus = digitalRead(RAIN_PIN);

  // ---------- PHÁT HIỆN MƯA ----------
  if (rainStatus == LOW)
  {
    if (!isRaining)
    {
      isRaining = true;
      rainStopTime = 0;    // Reset thời gian chờ
      servoRoof.write(90); // Đóng mái che
      logSys("MUA: Dong mai che");
    }
  }

  // ---------- KHÔNG MƯA ----------
  else
  { // rainStatus == HIGH
    if (isRaining && rainStopTime == 0)
    {
      // Ghi nhận thời điểm vừa hết mưa
      rainStopTime = millis();
    }

    // Sau 5 giây vẫn không mưa -> mở mái
    if (isRaining && rainStopTime > 0 &&
        millis() - rainStopTime >= RAIN_DELAY)
    {

      isRaining = false;
      rainStopTime = 0;
      servoRoof.write(0); // Mở mái che
      logSys("NANG: Mo mai che sau 5s");
    }
  }

  // ============================================
  // ===== LOGIC LỬA (Còi ngay) ========
  if (digitalRead(FIRE_PIN))
  {
    // 1. Phát hiện lửa -> Kích hoạt chế độ NGAY
    if (!fireMode)
    {
      fireMode = true;
      fireStart = millis(); // Bắt đầu đếm giờ mở cổng
      logDanger("Phat hien LUA!");
    }

    // 2. Chỉ mở cổng khi đã cháy > 3 giây
    if (millis() - fireStart > 3000)
    {
      lcd.clear();
      lcd.print("!!! CHAY !!!");
      servoIn.write(90);
      servoOut.write(90);
    }

    fireClear = 0; // Reset đếm tắt
  }
  else
  {
    // Không thấy lửa
    fireStart = 0;
    if (fireMode)
    {
      if (fireClear == 0)
        fireClear = millis();
      // Hết lửa 3 giây thì tắt chế độ (tắt còi, đóng cổng)
      if (millis() - fireClear > 3000)
      {
        fireMode = false;
        servoIn.write(0);
        servoOut.write(0);
        logSys("CHAY: Da tat");
        showHome();
      }
    }
  }

  // --- GAS ---
  if (digitalRead(GAS_PIN))
  {
    if (!gasMode)
    {
      gasMode = true;
      gasStart = millis();
      lcd.clear();
      lcd.print("Canh bao GAS");
      logDanger("Phat hien khi GAS");
    }
    if (millis() - gasStart > 4000)
    {
      lcd.clear();
      lcd.print("!!! GAS !!!");
      servoIn.write(90);
      servoOut.write(90);
    }
    gasClear = 0;
  }
  else if (gasMode)
  {
    if (gasClear == 0)
      gasClear = millis();
    if (millis() - gasClear > 3000)
    {
      gasMode = false;
      servoIn.write(0);
      servoOut.write(0);
      showHome();
    }
  }

  // ===========================================
  // ===== XỬ LÝ CÒI (PASSIVE BUZZER) ==========
  // ===========================================
  if (fireMode || gasMode)
  {
    if (millis() - beepTimer > 200)
    {
      beepTimer = millis();
      beepState = !beepState;
      if (beepState)
      {
        // Kêu tần số 2000Hz (Tiếng Tít)
        tone(BUZZER_PIN, 2000);
      }
      else
      {
        // Tắt tiếng
        noTone(BUZZER_PIN);
      }
    }
  }
  else
  {
    noTone(BUZZER_PIN); // Đảm bảo tắt còi khi an toàn
  }
  // ===========================================

  if (fireMode || gasMode)
    return;
  long dIn = getDistance(TRIG_IN, ECHO_IN);
  long dOut = getDistance(TRIG_OUT, ECHO_OUT);

  // ===== PHÁT HIỆN XE =====
  if (!waitingInput && !gateOpen)
  {
    if (dIn < DIST_THRESHOLD)
    {
      if (countCars() >= MAX_CARS)
      {
        lcd.clear();
        lcd.print("NHA XE DAY");
        delay(2000);
        showHome();
        return;
      }
      gateIsIn = true;
    }
    else if (dOut < DIST_THRESHOLD)
    {
      gateIsIn = false;
    }
    else
      return;

    waitingInput = true;
    inputID = "";
    inputTimer = millis();
    lcd.clear();
    lcd.print(gateIsIn ? "CONG VAO" : "CONG RA");
    lcd.setCursor(0, 1);
    lcd.print("Nhap TK");
    return;
  }

  // ===== NHẬP TK =====
  if (waitingInput)
  {
    char k = scanKeypad();
    if (k)
    {
      if (k >= '0' && k <= '9')
      {
        inputID += k;
        lcd.print("*");
      }
      if (k == '*')
      {
        inputID = "";
        lcd.setCursor(0, 1);
        lcd.print("Nhap TK      ");
      }
      if (k == '#')
      {
        if (gateIsIn)
        {
          // --- LOGIC XE VÀO ---
          if (!addCar(inputID))
          {
            lcd.clear();
            lcd.print("TK TON TAI");
            delay(1500);
            showHome();
            return;
          }
          servoIn.write(90);
          gateOpen = true;
          logSys("Mo cong VAO"); // Log hệ thống
        }
        else
        {
          // --- LOGIC XE RA (GIỮ NGUYÊN CODE TÍNH TIỀN) ---
          int i = findCar(inputID);
          if (i == -1)
          {
            lcd.clear();
            lcd.print("SAI TK");
            delay(1500);
            showHome();
            return;
          }

          // 1. Tính thời gian (phút)
          unsigned long duration = millis() - carTimeIn[i];
          long minutes = duration / 60000;
          if (minutes < 1)
            minutes = 1; // Đi dưới 1p vẫn tính 1p

          // 2. Tính tiền thô
          long rawFee = (minutes * PRICE_PER_HOUR) / 60;

          // 3. Làm tròn lên hàng nghìn
          long finalFee = ((rawFee + 999) / 1000) * 1000;

          // 4. Hiển thị thông tin
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("TG gui: ");
          lcd.print(minutes);
          lcd.print("p");

          lcd.setCursor(0, 1);
          lcd.print(finalFee);
          lcd.print("d A=Tra");

          // 5. Chờ thanh toán
          // QUAN TRỌNG: Thêm server.handleClient() vào đây để web không bị treo khi chờ trả tiền
          while (scanKeypad() != 'A')
          {
            server.handleClient();
          }

          // GHI LOG WEB KHI ĐÃ TRẢ TIỀN
          logCar("RA: TK " + inputID + " (" + String(finalFee) + "d)"); // Log xe ra
          logSys("Mo cong RA");                                         // Log hệ thống
          carID[i] = "";
          servoOut.write(90);
          gateOpen = true;
        }
        waitingInput = false;
      }
    }
    if (millis() - inputTimer > PASSWORD_TIMEOUT)
    {
      showHome();
    }
  }

  // ===== ĐÓNG BARIE =====
  if (gateOpen)
  {
    long d = gateIsIn ? dIn : dOut;
    if (d > DIST_THRESHOLD)
    {
      if (gateTimer == 0)
        gateTimer = millis();
      if (millis() - gateTimer > GATE_CLOSE_DELAY)
      {
        servoIn.write(0);
        servoOut.write(0);
        gateOpen = false;
        gateTimer = 0;
        showHome();
      }
    }
    else
      gateTimer = 0;
  }
}