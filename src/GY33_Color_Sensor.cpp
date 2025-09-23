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

  // Thiết lập độ sáng LED mặc định theo datasheet
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
  return Wire.read();
}

uint16_t GY33_Color_Sensor::read16Bit(uint8_t reg) {
  Wire.beginTransmission(i2c_address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(i2c_address, 2);
  uint16_t value = Wire.read() << 8;
  value |= Wire.read();
  return value;
}

void GY33_Color_Sensor::write8Bit(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(i2c_address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void GY33_Color_Sensor::setLEDBrightness(uint8_t level) {
  level = map(level, 0, 10, 10, 0); // Đảm bảo level trong khoảng 0-10
  if (level > 10) level = 10;

  // Đọc giá trị config hiện tại
  uint8_t config = read8Bit(REG_CONFIG);

  // Xóa bits độ sáng hiện tại (bits 7-4)
  config &= 0x0F;

  // Thiết lập độ sáng mới (dịch level sang bits 7-4)
  // THEO DATASHEET: level càng nhỏ → LED càng sáng
  config |= (level << 4);

  // Ghi giá trị config mới
  write8Bit(REG_CONFIG, config);

  led_brightness = level;
}

void GY33_Color_Sensor::whiteBalanceCalibration() {
  // Kích hoạt cân bằng trắng (set bit 0 của config register)
  uint8_t config = read8Bit(REG_CONFIG);
  config |= 0x01; // Set bit 0
  write8Bit(REG_CONFIG, config);

  delay(1000); // Đợi hiệu chỉnh hoàn tất

  // Tắt chế độ hiệu chỉnh
  config &= ~0x01; // Clear bit 0
  write8Bit(REG_CONFIG, config);
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

  // Hiển thị độ sáng LED (theo datasheet)
  Serial.print("Độ sáng LED (0-10, 0=sáng nhất): ");
  Serial.println(led_brightness);

  Serial.println("==============================================");
}

void GY33_Color_Sensor::printHelp() {
  Serial.println("===== TRỢ GIÚP LỆNH =====");
  Serial.println("b - Tăng độ sáng LED (giá trị số giảm, LED sáng hơn)");
  Serial.println("d - Giảm độ sáng LED (giá trị số tăng, LED tối hơn)");
  Serial.println("w - Hiệu chỉnh cân bằng trắng");
  Serial.println("? - Hiển thị trợ giúp này");
  Serial.println("LƯU Ý: Giá trị độ sáng 0 = sáng nhất, 10 = tối nhất");
  Serial.println("=========================");
}