#include "GY33_Color_Sensor.h"

GY33_Color_Sensor::GY33_Color_Sensor(uint8_t address)
    : i2c_address(address), led_brightness(3) {
  // Khởi tạo các thành viên dữ liệu
  raw_red = raw_green = raw_blue = raw_clear = 0;
  lux = color_temp = 0;
  r = g = b = color_value = 0;
}

bool GY33_Color_Sensor::begin(uint8_t sda, uint8_t scl) {
  Wire.begin(sda, scl);
  delay(100); // Đợi một chút để cảm biến khởi động

  // Kiểm tra xem cảm biến có kết nối không
  if (!isConnected()) {
    return false;
  }

  // Thiết lập độ sáng LED mặc định
  setLEDBrightness(led_brightness);

  return true;
}

bool GY33_Color_Sensor::isConnected() {
  Wire.beginTransmission(i2c_address);
  return (Wire.endTransmission() == 0);
}

uint8_t GY33_Color_Sensor::read8Bit(uint8_t reg) {
  Wire.beginTransmission(i2c_address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(i2c_address, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

uint16_t GY33_Color_Sensor::read16Bit(uint8_t reg) {
  Wire.beginTransmission(i2c_address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(i2c_address, 2);
  if (Wire.available() >= 2) {
    uint16_t value = Wire.read() << 8;
    value |= Wire.read();
    return value;
  }
  return 0;
}

void GY33_Color_Sensor::write8Bit(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(i2c_address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void GY33_Color_Sensor::setLEDBrightness(uint8_t level) {
  level = map(level, 0, 10, 10, 0); // Giới hạn mức độ sáng từ 0-10
  if (level > 10) level = 10;

  // Đọc giá trị config hiện tại
  uint8_t config = read8Bit(REG_CONFIG);

  // Xóa bits độ sáng hiện tại (bits 7-4)
  config &= 0x0F;

  // Thiết lập độ sáng mới (dịch level sang bits 7-4)
  config |= (level << 4);

  // Ghi giá trị config mới
  write8Bit(REG_CONFIG, config);

  led_brightness = level;
}

bool GY33_Color_Sensor::whiteBalanceCalibration(bool verbose) {
  if (verbose) {
    Serial.println("🚀 Bắt đầu hiệu chỉnh cân bằng trắng...");
  }

  // Bước 1: Đọc giá trị ban đầu
  readAllData();
  if (verbose) {
    Serial.print("📊 Giá trị trước hiệu chỉnh - R:");
    Serial.print(raw_red);
    Serial.print(" G:");
    Serial.print(raw_green);
    Serial.print(" B:");
    Serial.println(raw_blue);
  }

  // Bước 2: Kiểm tra điều kiện ánh sáng
  if (raw_clear < 100) { // Ngưỡng tối thiểu
    if (verbose) {
      Serial.println("❌ Ánh sáng không đủ cho hiệu chỉnh! (Clear < 100)");
    }
    return false;
  }

  // Bước 3: Kiểm tra sự chênh lệch màu quá lớn
  uint16_t max_val = max(raw_red, max(raw_green, raw_blue));
  uint16_t min_val = min(raw_red, min(raw_green, raw_blue));

  if (max_val > 0 && (max_val - min_val) * 10 / max_val > 8) {
    if (verbose) {
      Serial.println("❌ Vật thể không phải màu trắng chuẩn!");
    }
    return false;
  }

  // Bước 4: Kích hoạt chế độ hiệu chuẩn
  uint8_t config = read8Bit(REG_CONFIG);
  config |= 0x01; // Set bit 0 - White Balance Enable
  write8Bit(REG_CONFIG, config);

  if (verbose) {
    Serial.println("⏳ Đang hiệu chỉnh... (1.5 giây)");
  }

  // Bước 5: Chờ hiệu chỉnh hoàn tất
  delay(1500); // Thời gian theo datasheet

  // Bước 6: Tắt chế độ hiệu chuẩn
  config &= ~0x01; // Clear bit 0
  write8Bit(REG_CONFIG, config);

  // Bước 7: Xác minh kết quả
  delay(100);
  readAllData();

  if (verbose) {
    Serial.print("✅ Giá trị sau hiệu chỉnh - R:");
    Serial.print(raw_red);
    Serial.print(" G:");
    Serial.print(raw_green);
    Serial.print(" B:");
    Serial.println(raw_blue);
  }

  // Bước 8: Kiểm tra kết quả (R≈G≈B cho vật thể trắng)
  if (raw_green == 0) raw_green = 1; // Tránh chia cho 0
  if (raw_blue == 0) raw_blue = 1;

  float ratio_rg = (float)raw_red / raw_green;
  float ratio_rb = (float)raw_red / raw_blue;
  float ratio_gb = (float)raw_green / raw_blue;

  bool success = (ratio_rg > 0.7f && ratio_rg < 1.3f &&
                  ratio_rb > 0.7f && ratio_rb < 1.3f &&
                  ratio_gb > 0.7f && ratio_gb < 1.3f);

  if (verbose) {
    if (success) {
      Serial.println("🎯 Hiệu chỉnh thành công! Các giá trị RGB cân bằng.");
    } else {
      Serial.println("⚠️ Hiệu chỉnh có thể chưa tối ưu");
      Serial.print("Tỷ lệ R/G: ");
      Serial.print(ratio_rg, 2);
      Serial.print(", R/B: ");
      Serial.print(ratio_rb, 2);
      Serial.print(", G/B: ");
      Serial.println(ratio_gb, 2);
    }
  }

  return success;
}

void GY33_Color_Sensor::autoWhiteBalanceWithFeedback() {
  Serial.println("\n🔧 CHẾ ĐỘ HIỆU CHUẨN CÂN BẰNG TRẮNG");
  Serial.println("==========================================");
  Serial.println("📋 HƯỚNG DẪN:");
  Serial.println("1. Đặt cảm biến hướng về vật thể màu TRẮNG");
  Serial.println("2. Đảm bảo ánh sáng ổn định và đủ mạnh");
  Serial.println("3. Vật thể trắng nên chiếm toàn bộ vùng cảm biến");
  Serial.println("4. Giữ khoảng cách 2-5cm");
  Serial.println();
  Serial.println("⌨️ LỆNH:");
  Serial.println("c - Bắt đầu hiệu chuẩn");
  Serial.println("x - Hủy bỏ");
  Serial.println("? - Hiển thị hướng dẫn này");
  Serial.println("==========================================");

  bool waitingForCommand = true;

  while (waitingForCommand) {
    if (Serial.available()) {
      char cmd = Serial.read();

      switch (cmd) {
      case 'c': // Bắt đầu hiệu chuẩn
      {
        Serial.println("\n🎯 Bắt đầu hiệu chuẩn...");

        // Kiểm tra ánh sáng trước
        readAllData();
        Serial.print("💡 Cường độ ánh sáng (Clear): ");
        Serial.println(raw_clear);

        if (raw_clear < 100) {
          Serial.println("❌ Ánh sáng quá yếu! Vui lòng cải thiện ánh sáng.");
          Serial.println("💡 Gợi ý: Đưa cảm biến gần hơn hoặc tăng cường độ sáng");
          break;
        }

        Serial.println("⏳ Đang thực hiện hiệu chuẩn...");

        if (whiteBalanceCalibration(true)) {
          Serial.println("💾 Hiệu chỉnh đã được lưu thành công!");
          Serial.println("✅ Có thể sử dụng cảm biến ngay bây giờ.");
        } else {
          Serial.println("❌ Hiệu chỉnh thất bại! Vui lòng thử lại.");
          Serial.println("💡 Gợi ý: Kiểm tra vật thể trắng và ánh sáng");
        }

        waitingForCommand = false;
      } break;

      case 'x': // Hủy bỏ
        Serial.println("❌ Hủy hiệu chỉnh");
        waitingForCommand = false;
        break;

      case '?': // Hiển thị hướng dẫn
        Serial.println("\n🔧 CHẾ ĐỘ HIỆU CHUẨN CÂN BẰNG TRẮNG");
        Serial.println("==========================================");
        Serial.println("📋 HƯỚNG DẪN:");
        Serial.println("1. Đặt cảm biến hướng về vật thể màu TRẮNG");
        Serial.println("2. Đảm bảo ánh sáng ổn định và đủ mạnh");
        Serial.println("3. Vật thể trắng nên chiếm toàn bộ vùng cảm biến");
        Serial.println("4. Giữ khoảng cách 2-5cm");
        Serial.println();
        Serial.println("⌨️ LỆNH:");
        Serial.println("c - Bắt đầu hiệu chuẩn");
        Serial.println("x - Hủy bỏ");
        Serial.println("? - Hiển thị hướng dẫn này");
        Serial.println("==========================================");
        break;

      default:
        // Bỏ qua các ký tự không hợp lệ
        break;
      }
    }

    delay(100); // Tránh chiếm CPU
  }

  Serial.println("🔚 Thoát chế độ hiệu chuẩn");
}

void GY33_Color_Sensor::readAllData() {
  // Đọc dữ liệu RGBC raw
  raw_red = read16Bit(REG_RAW_RED_H);
  raw_green = read16Bit(REG_RAW_GREEN_H);
  raw_blue = read16Bit(REG_RAW_BLUE_H);
  raw_clear = read16Bit(REG_RAW_CLEAR_H);

  // Đọc giá trị Lux và nhiệt độ màu
  lux = read16Bit(REG_LUX_H);
  color_temp = read16Bit(REG_CT_H);

  // Đọc giá trị RGB đã xử lý
  r = read8Bit(REG_R);
  g = read8Bit(REG_G);
  b = read8Bit(REG_B);

  // Đọc giá trị màu đơn giản
  color_value = read8Bit(REG_COLOR);
}

String GY33_Color_Sensor::getColorName() const {
  // Chuyển đổi giá trị màu thành tên màu dựa trên bit
  if (color_value & 0x01) return "Đỏ";
  if (color_value & 0x02) return "Vàng";
  if (color_value & 0x04) return "Hồng";
  if (color_value & 0x08) return "Trắng";
  if (color_value & 0x10) return "Đen";
  if (color_value & 0x20) return "Xanh lá";
  if (color_value & 0x40) return "Xanh dương đậm";
  if (color_value & 0x80) return "Xanh dương";

  return "Không xác định";
}

void GY33_Color_Sensor::printData() {
  Serial.println("========== DỮ LIỆU CẢM BIẾN MÀU GY-33 ==========");

  // Hiển thị giá trị RGBC raw
  Serial.print("RGBC Raw - R:");
  Serial.print(raw_red);
  Serial.print(" G:");
  Serial.print(raw_green);
  Serial.print(" B:");
  Serial.print(raw_blue);
  Serial.print(" C:");
  Serial.println(raw_clear);

  // Hiển thị giá trị Lux và nhiệt độ màu
  Serial.print("Ánh sáng: ");
  Serial.print(lux);
  Serial.print(" lux | Nhiệt độ màu: ");
  Serial.print(color_temp);
  Serial.println("K");

  // Hiển thị giá trị RGB đã xử lý
  Serial.print("RGB - R:");
  Serial.print(r);
  Serial.print(" G:");
  Serial.print(g);
  Serial.print(" B:");
  Serial.println(b);

  // Hiển thị màu nhận diện được
  Serial.print("Màu nhận diện: ");
  Serial.print(getColorName());
  Serial.print(" (0x");
  Serial.print(color_value, HEX);
  Serial.println(")");

  // Hiển thị độ sáng LED hiện tại
  Serial.print("Độ sáng LED: ");
  Serial.println(led_brightness);

  Serial.println("==============================================");
}

void GY33_Color_Sensor::printHelp() {
  Serial.println("===== TRỢ GIÚP LỆNH GY-33 =====");
  Serial.println("b     - Tăng độ sáng LED");
  Serial.println("d     - Giảm độ sáng LED");
  Serial.println("w     - Hiệu chỉnh cân bằng trắng (tự động)");
  Serial.println("w1    - Hiệu chỉnh cân bằng trắng (chi tiết)");
  Serial.println("?     - Hiển thị trợ giúp này");
  Serial.println("================================");
}