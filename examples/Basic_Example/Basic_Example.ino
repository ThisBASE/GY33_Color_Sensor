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
}

void loop() {
  // Đọc tất cả dữ liệu từ cảm biến
  colorSensor.readAllData();

  // Hiển thị dữ liệu
  colorSensor.printData();

  delay(1000); // Đọc dữ liệu mỗi giây
}