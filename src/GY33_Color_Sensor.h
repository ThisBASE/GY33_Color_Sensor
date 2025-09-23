#ifndef GY33_COLOR_SENSOR_H
#define GY33_COLOR_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

// Địa chỉ I2C mặc định của GY-33 (7-bit address là 0x5A)
#define GY33_I2C_ADDRESS 0x5A

class GY33_Color_Sensor {
public:
  // Constructor
  GY33_Color_Sensor(uint8_t address = GY33_I2C_ADDRESS);

  // Khởi tạo cảm biến
  bool begin(uint8_t sda = SDA, uint8_t scl = SCL);

  // Kiểm tra kết nối
  bool isConnected();

  // Cấu hình
  void setLEDBrightness(uint8_t level);
  void whiteBalanceCalibration();

  // Đọc dữ liệu
  void readAllData();

  // Truy cập dữ liệu
  uint16_t getRawRed() const { return raw_red; }
  uint16_t getRawGreen() const { return raw_green; }
  uint16_t getRawBlue() const { return raw_blue; }
  uint16_t getRawClear() const { return raw_clear; }
  uint16_t getLux() const { return lux; }
  uint16_t getColorTemperature() const { return color_temp; }
  uint8_t getR() const { return r; }
  uint8_t getG() const { return g; }
  uint8_t getB() const { return b; }
  uint8_t getColorValue() const { return color_value; }
  uint8_t getLEDBrightness() const { return led_brightness; }
  String getColorName() const;

  // Tiện ích
  void printData();
  static void printHelp();

private:
  // Giao tiếp I2C
  uint8_t read8Bit(uint8_t reg);
  uint16_t read16Bit(uint8_t reg);
  void write8Bit(uint8_t reg, uint8_t value);

  // Dữ liệu cảm biến
  uint16_t raw_red, raw_green, raw_blue, raw_clear;
  uint16_t lux, color_temp;
  uint8_t r, g, b, color_value;
  uint8_t led_brightness;
  uint8_t i2c_address;

  // Định nghĩa các thanh ghi
  enum Registers {
    REG_RAW_RED_H = 0x00,
    REG_RAW_RED_L = 0x01,
    REG_RAW_GREEN_H = 0x02,
    REG_RAW_GREEN_L = 0x03,
    REG_RAW_BLUE_H = 0x04,
    REG_RAW_BLUE_L = 0x05,
    REG_RAW_CLEAR_H = 0x06,
    REG_RAW_CLEAR_L = 0x07,
    REG_LUX_H = 0x08,
    REG_LUX_L = 0x09,
    REG_CT_H = 0x0A,
    REG_CT_L = 0x0B,
    REG_R = 0x0C,
    REG_G = 0x0D,
    REG_B = 0x0E,
    REG_COLOR = 0x0F,
    REG_CONFIG = 0x10
  };
};

#endif // GY33_COLOR_SENSOR_H