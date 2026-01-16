#include "PMS.h"

// We use Serial2 for the sensor. 
// On most ESP32 boards, Serial2 is on GPIO 16 (RX) and 17 (TX).
PMS pms(Serial2);
PMS::DATA data;

void setup() {
  // Start the USB connection for the computer monitor
  Serial.begin(115200);
  
  // Start the connection to the PMS sensor
  // Parameters: Baud Rate, Serial Mode, RX Pin, TX Pin
  Serial2.begin(9600, SERIAL_8N1, 16, 17); 

  Serial.println("\n--- ESP32 + PMS7003 Test ---");
  Serial.println("Waiting for data...");
}

void loop() {
  if (pms.read(data)) {
    Serial.print("PM 1.0: ");
    Serial.print(data.PM_AE_UG_1_0);
    
    Serial.print("\tPM 2.5: ");
    Serial.print(data.PM_AE_UG_2_5);
    
    Serial.print("\tPM 10.0: ");
    Serial.println(data.PM_AE_UG_10_0);
  }
}