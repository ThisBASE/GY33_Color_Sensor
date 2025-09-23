#include <GY33_Color_Sensor.h>

GY33_Color_Sensor colorSensor;

void setup() {
  Serial.begin(115200);

  // Khởi tạo cảm biến
  if (!colorSensor.begin()) {
    Serial.println("Lỗi: Không tìm thấy GY-33! Kiểm tra kết nối.");
    while (1)
      ; // Dừng chương trình nếu không kết nối được
  }

  Serial.println("Đã kết nối với GY-33 qua I2C");

  // Thiết lập độ sáng LED mặc định
  colorSensor.setLEDBrightness(3);

  // Hiển thị trợ giúp
  GY33_Color_Sensor::printHelp();
}

void loop() {
  // Đọc tất cả dữ liệu từ cảm biến
  colorSensor.readAllData();

  // Hiển thị dữ liệu
  colorSensor.printData();

  // Xử lý lệnh từ Serial
  if (Serial.available()) {
    char command = Serial.read();

    switch (command) {
    case 'b': // Tăng độ sáng LED
      if (colorSensor.getLEDBrightness() < 10) {
        colorSensor.setLEDBrightness(colorSensor.getLEDBrightness() + 1);
        Serial.print("Đã thiết lập độ sáng LED: ");
        Serial.println(colorSensor.getLEDBrightness());
      }
      break;

    case 'd': // Giảm độ sáng LED
      if (colorSensor.getLEDBrightness() > 0) {
        colorSensor.setLEDBrightness(colorSensor.getLEDBrightness() - 1);
        Serial.print("Đã thiết lập độ sáng LED: ");
        Serial.println(colorSensor.getLEDBrightness());
      }
      break;

    case 'w': // Hiệu chỉnh cân bằng trắng
      Serial.println("Bắt đầu hiệu chỉnh cân bằng trắng...");
      Serial.println("Đặt cảm biến hướng tới vật thể màu trắng và nhấn phím bất kỳ để tiếp tục");
      while (!Serial.available())
        ;
      Serial.read(); // Xóa buffer

      colorSensor.whiteBalanceCalibration();
      Serial.println("Hiệu chỉnh cân bằng trắng hoàn tất!");
      break;

    case '?': // Hiển thị trợ giúp
      GY33_Color_Sensor::printHelp();
      break;

    default:
      break;
    }
  }

  delay(1000); // Đọc dữ liệu mỗi giây
}