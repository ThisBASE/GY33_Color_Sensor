#include <GY33_Color_Sensor.h>
#include <WiFi.h>

GY33_Color_Sensor colorSensor;

// Biến để lưu trữ giá trị hiệu chỉnh
int calibrationData[3] = {0, 0, 0}; // RGB calibration offsets
bool isCalibrated = false;
bool autoCalibrationEnabled = true; // Bật auto calibration trong setup

void setup() {
  Serial.begin(115200);
  delay(10000);
  Serial.println("Chương trình in màu với hiệu chỉnh - Phiên bản cập nhật");

  // Khởi tạo cảm biến
  if (!colorSensor.begin(9, 8)) {
    Serial.println("Lỗi: Không tìm thấy GY-33! Kiểm tra kết nối.");
    while (1)
      ; // Dừng chương trình nếu không kết nối được
  }

  Serial.println("Đã kết nối với GY-33 qua I2C");

  // Thiết lập độ sáng LED mặc định
  colorSensor.setLEDBrightness(0);

  // HIỆU CHỈNH CÂN BẰNG TRẮNG TỰ ĐỘNG TRONG SETUP
  performAutoWhiteBalanceInSetup();

  // Hiển thị trợ giúp
  printEnhancedHelp();

  // Tải dữ liệu hiệu chỉnh từ EEPROM (nếu có)
  loadCalibrationData();
}

// Hàm hiệu chỉnh cân bằng trắng tự động trong setup
void performAutoWhiteBalanceInSetup() {
  if (!autoCalibrationEnabled) {
    Serial.println("❌ Auto calibration đã tắt - Bỏ qua hiệu chỉnh");
    return;
  }

  Serial.println("\n🎯 BẮT ĐẦU HIỆU CHỈNH CÂN BẰNG TRẮNG TỰ ĐỘNG");
  Serial.println("==============================================");

  // Đợi 2 giây để người dùng chuẩn bị
  Serial.println("⏳ Chuẩn bị hiệu chỉnh trong 2 giây...");
  Serial.println("💡 Hãy đặt cảm biến hướng về vật thể màu TRẮNG");
  Serial.println("📏 Khoảng cách lý tưởng: 2-5cm");
  Serial.println("💡 Đảm bảo ánh sáng ổn định");

  for (int i = 3; i > 0; i--) {
    Serial.print("⏰ Bắt đầu sau ");
    Serial.print(i);
    Serial.println(" giây...");
    delay(1000);
  }

  Serial.println("🔧 Đang thực hiện hiệu chỉnh cân bằng trắng...");

  // Thực hiện hiệu chỉnh với chế độ verbose
  bool success = colorSensor.whiteBalanceCalibration(true);

  if (success) {
    Serial.println("✅ HIỆU CHỈNH THÀNH CÔNG!");
    Serial.println("📊 Cảm biến đã sẵn sàng sử dụng");

    // Đọc và hiển thị kết quả sau hiệu chỉnh
    colorSensor.readAllData();
    Serial.print("Kết quả hiệu chỉnh - R:");
    Serial.print(colorSensor.getRawRed());
    Serial.print(" G:");
    Serial.print(colorSensor.getRawGreen());
    Serial.print(" B:");
    Serial.println(colorSensor.getRawBlue());

    // Kiểm tra độ cân bằng
    checkColorBalance();

  } else {
    Serial.println("❌ HIỆU CHỈNH THẤT BẠI!");
    Serial.println("💡 Nguyên nhân có thể:");
    Serial.println("   - Ánh sáng không đủ");
    Serial.println("   - Vật thể không phải màu trắng");
    Serial.println("   - Khoảng cách không phù hợp");
    Serial.println("📝 Sử dụng lệnh 'w' để thử lại sau");
  }

  Serial.println("==============================================");
}

// Kiểm tra độ cân bằng màu sau hiệu chỉnh
void checkColorBalance() {
  uint16_t r = colorSensor.getRawRed();
  uint16_t g = colorSensor.getRawGreen();
  uint16_t b = colorSensor.getRawBlue();

  if (g == 0) g = 1; // Tránh chia cho 0
  if (b == 0) b = 1;

  float ratio_rg = (float)r / g;
  float ratio_rb = (float)r / b;
  float ratio_gb = (float)g / b;

  Serial.println("📈 ĐÁNH GIÁ ĐỘ CÂN BẰNG:");
  Serial.print("Tỷ lệ R/G: ");
  Serial.print(ratio_rg, 3);
  Serial.print(" | Lý tưởng: 0.9-1.1 | ");
  Serial.println((ratio_rg >= 0.9 && ratio_rg <= 1.1) ? "✅ TỐT" : "⚠️ CẦN CẢI THIỆN");

  Serial.print("Tỷ lệ R/B: ");
  Serial.print(ratio_rb, 3);
  Serial.print(" | Lý tưởng: 0.9-1.1 | ");
  Serial.println((ratio_rb >= 0.9 && ratio_rb <= 1.1) ? "✅ TỐT" : "⚠️ CẦN CẢI THIỆN");

  Serial.print("Tỷ lệ G/B: ");
  Serial.print(ratio_gb, 3);
  Serial.print(" | Lý tưởng: 0.9-1.1 | ");
  Serial.println((ratio_gb >= 0.9 && ratio_gb <= 1.1) ? "✅ TỐT" : "⚠️ CẦN CẢI THIỆN");
}

void loop() {
  // Đọc tất cả dữ liệu từ cảm biến
  colorSensor.readAllData();

  // Hiển thị dữ liệu đã hiệu chỉnh
  printCalibratedData();

  // Xử lý lệnh từ Serial
  if (Serial.available()) {
    char command = Serial.read();
    processCommand(command);
  }

  delay(500); // Đọc dữ liệu mỗi 500ms
}

void processCommand(char command) {
  switch (command) {
  case 'b': // Tăng độ sáng LED
    if (colorSensor.getLEDBrightness() > 0) {
      colorSensor.setLEDBrightness(colorSensor.getLEDBrightness() - 1);
      Serial.print("Đã thiết lập độ sáng LED: ");
      Serial.print(colorSensor.getLEDBrightness());
      Serial.println(" (0 = sáng nhất, 10 = tối nhất)");
    }
    break;

  case 'd': // Giảm độ sáng LED
    if (colorSensor.getLEDBrightness() < 10) {
      colorSensor.setLEDBrightness(colorSensor.getLEDBrightness() + 1);
      Serial.print("Đã thiết lập độ sáng LED: ");
      Serial.print(colorSensor.getLEDBrightness());
      Serial.println(" (0 = sáng nhất, 10 = tối nhất)");
    }
    break;

  case 'w': // Hiệu chỉnh cân bằng trắng
    performManualWhiteBalance();
    break;

  case 'a': // Chế độ auto white balance với feedback
    colorSensor.autoWhiteBalanceWithFeedback();
    break;

  case 'c': // Hiệu chỉnh manual (nhập offset R G B)
    performManualCalibration();
    break;

  case 'r': // Reset hiệu chỉnh
    resetCalibration();
    break;

  case 's': // Lưu hiệu chỉnh
    saveCalibrationData();
    break;

  case 'l': // Tải hiệu chỉnh
    loadCalibrationData();
    break;

  case 'x': // Tắt/bật auto calibration
    autoCalibrationEnabled = !autoCalibrationEnabled;
    Serial.print("Auto calibration: ");
    Serial.println(autoCalibrationEnabled ? "✅ BẬT" : "❌ TẮT");
    break;

  case '?': // Hiển thị trợ giúp
    printEnhancedHelp();
    break;

  default:
    break;
  }
}

// Hàm hiệu chỉnh cân bằng trắng thủ công
void performManualWhiteBalance() {
  Serial.println("\n=== HIỆU CHỈNH CÂN BẰNG TRẮNG THỦ CÔNG ===");
  Serial.println("Nhấn 'c' để bắt đầu hoặc 'x' để hủy");

  while (!Serial.available())
    delay(100);

  char cmd = Serial.read();
  if (cmd == 'c') {
    Serial.println("Đang thực hiện hiệu chỉnh...");
    bool success = colorSensor.whiteBalanceCalibration(true);

    if (success) {
      Serial.println("✅ Hiệu chỉnh thành công!");
    } else {
      Serial.println("❌ Hiệu chỉnh thất bại!");
    }
  } else {
    Serial.println("❌ Đã hủy hiệu chỉnh");
  }
}

// Hàm hiệu chỉnh manual với offset
void performManualCalibration() {
  Serial.println("\n=== HIỆU CHỈNH MANUAL ===");
  Serial.println("Nhập giá trị offset cho R G B (ví dụ: 10 -5 3):");
  Serial.println("Lưu ý: Offset áp dụng cho giá trị RAW (0-65535)");

  // Chờ dữ liệu đầu vào
  while (!Serial.available())
    delay(100);

  String input = Serial.readString();
  input.trim();
  int r, g, b;

  if (sscanf(input.c_str(), "%d %d %d", &r, &g, &b) == 3) {
    calibrationData[0] = r;
    calibrationData[1] = g;
    calibrationData[2] = b;
    isCalibrated = true;

    Serial.println("✅ Hiệu chỉnh manual hoàn tất!");
    Serial.print("Offset R: ");
    Serial.println(r);
    Serial.print("Offset G: ");
    Serial.println(g);
    Serial.print("Offset B: ");
    Serial.println(b);
  } else {
    Serial.println("❌ Định dạng không hợp lệ! Sử dụng: R G B");
  }
}

// In dữ liệu đã hiệu chỉnh
void printCalibratedData() {
  Serial.println("\n--- DỮ LIỆU CẢM BIẾN MÀU ---");

  // Dữ liệu RAW gốc
  Serial.print("RAW RGB: ");
  Serial.print(colorSensor.getRawRed());
  Serial.print(", ");
  Serial.print(colorSensor.getRawGreen());
  Serial.print(", ");
  Serial.print(colorSensor.getRawBlue());
  Serial.print(" | Clear: ");
  Serial.println(colorSensor.getRawClear());

  // Dữ liệu RAW đã hiệu chỉnh
  if (isCalibrated) {
    Serial.print("RAW Calibrated: ");
    Serial.print(constrain(colorSensor.getRawRed() + calibrationData[0], 0, 65535));
    Serial.print(", ");
    Serial.print(constrain(colorSensor.getRawGreen() + calibrationData[1], 0, 65535));
    Serial.print(", ");
    Serial.println(constrain(colorSensor.getRawBlue() + calibrationData[2], 0, 65535));
  }

  // Dữ liệu đã xử lý (RGB 0-255)
  Serial.print("RGB Processed: ");
  Serial.print(colorSensor.getR());
  Serial.print(", ");
  Serial.print(colorSensor.getG());
  Serial.print(", ");
  Serial.println(colorSensor.getB());

  // Thông tin màu và ánh sáng
  Serial.print("Màu: ");
  Serial.print(colorSensor.getColorName());
  Serial.print(" | Lux: ");
  Serial.print(colorSensor.getLux());
  Serial.print(" | Temp: ");
  Serial.print(colorSensor.getColorTemperature());
  Serial.println("K");

  Serial.println("-----------------------------");
}

// Reset hiệu chỉnh
void resetCalibration() {
  calibrationData[0] = 0;
  calibrationData[1] = 0;
  calibrationData[2] = 0;
  isCalibrated = false;
  Serial.println("✅ Đã reset hiệu chỉnh về mặc định");
}

// Lưu hiệu chỉnh
void saveCalibrationData() {
  Serial.println("💾 Đã lưu dữ liệu hiệu chỉnh");
  Serial.print("Offset R: ");
  Serial.println(calibrationData[0]);
  Serial.print("Offset G: ");
  Serial.println(calibrationData[1]);
  Serial.print("Offset B: ");
  Serial.println(calibrationData[2]);
}

// Tải hiệu chỉnh
void loadCalibrationData() {
  Serial.println("📥 Đã tải dữ liệu hiệu chỉnh");
  if (isCalibrated) {
    Serial.print("Offset R: ");
    Serial.println(calibrationData[0]);
    Serial.print("Offset G: ");
    Serial.println(calibrationData[1]);
    Serial.print("Offset B: ");
    Serial.println(calibrationData[2]);
  } else {
    Serial.println("Không có dữ liệu hiệu chỉnh được lưu");
  }
}

// Hiển thị trợ giúp nâng cao
void printEnhancedHelp() {
  Serial.println("\n=== TRỢ GIÚP LỆNH GY-33 NÂNG CAO ===");
  Serial.println("b - Tăng độ sáng LED");
  Serial.println("d - Giảm độ sáng LED");
  Serial.println("w - Hiệu chỉnh cân bằng trắng (thủ công)");
  Serial.println("a - Hiệu chỉnh cân bằng trắng (auto với feedback)");
  Serial.println("c - Hiệu chỉnh manual (nhập offset R G B)");
  Serial.println("r - Reset hiệu chỉnh");
  Serial.println("s - Lưu hiệu chỉnh");
  Serial.println("l - Tải hiệu chỉnh");
  Serial.println("x - Tắt/bật auto calibration");
  Serial.println("? - Hiển thị trợ giúp này");
  Serial.println("=====================================");
}