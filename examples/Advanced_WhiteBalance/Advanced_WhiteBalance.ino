#include <GY33_Color_Sensor.h>

GY33_Color_Sensor colorSensor;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("🎨 DEMO CÂN BẰNG TRẮNG GY-33");
  Serial.println("=============================");

  // Khởi tạo cảm biến
  if (!colorSensor.begin()) {
    Serial.println("❌ Không tìm thấy cảm biến GY-33!");
    while (1)
      ;
  }

  Serial.println("✅ Cảm biến GY-33 đã sẵn sàng");
  colorSensor.printHelp();
}

void loop() {
  // Đọc và hiển thị dữ liệu
  colorSensor.readAllData();
  colorSensor.printData();

  // Xử lý lệnh từ Serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "w") {
      // Chế độ hiệu chuẩn tự động với hướng dẫn
      colorSensor.autoWhiteBalanceWithFeedback();

    } else if (command == "w1") {
      // Chế độ hiệu chuẩn chi tiết (verbose)
      Serial.println("🔧 Chế độ hiệu chuẩn chi tiết...");
      bool success = colorSensor.whiteBalanceCalibration(true);
      if (success) {
        Serial.println("🎉 Hiệu chuẩn thành công!");
      } else {
        Serial.println("💥 Hiệu chuẩn thất bại!");
      }

    } else if (command == "b") {
      // Tăng độ sáng LED
      uint8_t current = 0; // Giả sử có hàm getLEDBrightness()
      colorSensor.setLEDBrightness(current + 1);
      Serial.print("💡 Độ sáng LED: ");
      Serial.println(current + 1);

    } else if (command == "d") {
      // Giảm độ sáng LED
      uint8_t current = 0; // Giả sử có hàm getLEDBrightness()
      if (current > 0) {
        colorSensor.setLEDBrightness(current - 1);
        Serial.print("💡 Độ sáng LED: ");
        Serial.println(current - 1);
      }

    } else if (command == "?") {
      colorSensor.printHelp();
    }
  }

  delay(2000); // Đọc dữ liệu mỗi 2 giây
}