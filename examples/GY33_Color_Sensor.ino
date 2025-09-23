#include "GY33_Color_Sensor.h"

GY33_Color_Sensor colorSensor;

void setup() {
  Serial.begin(115200);

  // Initialize the sensor
  if (!colorSensor.begin()) {
    Serial.println("Lỗi: Không tìm thấy GY-33! Kiểm tra kết nối.");
    while (1)
      ; // Stop program if not connected
  }

  Serial.println("Đã kết nối với GY-33 qua I2C");

  // Set default LED brightness
  colorSensor.setLEDBrightness(3);

  // Display help
  colorSensor.printHelp();
}

void loop() {
  // Read all data from sensor
  colorSensor.readAllData();

  // Display data
  colorSensor.printData();

  // Handle serial commands
  if (Serial.available()) {
    char command = Serial.read();

    switch (command) {
    case 'b': // Increase LED brightness
      colorSensor.setLEDBrightness(colorSensor.getLEDBrightness() + 1);
      Serial.print("Đã thiết lập độ sáng LED: ");
      Serial.println(colorSensor.getLEDBrightness());
      break;

    case 'd': // Decrease LED brightness
      colorSensor.setLEDBrightness(colorSensor.getLEDBrightness() - 1);
      Serial.print("Đã thiết lập độ sáng LED: ");
      Serial.println(colorSensor.getLEDBrightness());
      break;

    case 'w': // White balance calibration
      Serial.println("Bắt đầu hiệu chỉnh cân bằng trắng...");
      Serial.println("Đặt cảm biến hướng tới vật thể màu trắng và nhấn phím bất kỳ để tiếp tục");
      while (!Serial.available())
        ;
      Serial.read(); // Clear buffer

      colorSensor.whiteBalanceCalibration();
      Serial.println("Hiệu chỉnh cân bằng trắng hoàn tất!");
      break;

    case '?': // Display help
      colorSensor.printHelp();
      break;

    default:
      break;
    }
  }

  delay(1000); // Read data every second
}