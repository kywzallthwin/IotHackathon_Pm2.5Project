#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PMS.h"

// --- OLED CONFIGURATION ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
// ADDRESS IS SET TO 0x3C HERE
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- PMS SENSOR CONFIGURATION ---
// PMS TX connected to GPIO 16 (ESP32 RX2)
// PMS RX connected to GPIO 17 (ESP32 TX2)
PMS pms(Serial2);
PMS::DATA data;

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize PMS Sensor
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // 2. Initialize OLED
  // We use the address 0x3C found by your scanner
  // SCL	= GPIO 22	I2C Clock
  // SDA	= GPIO 21	I2C Data
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Screen failed to start - Check wires!"));
    for(;;); // Stop here if screen fails
  }
  
  // Show startup text
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Sensor warming up...");
  display.display();
  
  Serial.println("\n--- SYSTEM READY ---");
}

void loop() {
  if (pms.read(data)) {
    // --- PRINT TO SERIAL MONITOR ---
    Serial.print("PM 2.5: ");
    Serial.println(data.PM_AE_UG_2_5);

    // --- PRINT TO OLED SCREEN ---
    display.clearDisplay();
    
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Air Quality (ug/m3)");
    
    // PM 2.5 Big Number
    display.setTextSize(2); // Make it big
    display.setCursor(10, 20);
    display.print("PM2.5: ");
    display.print(data.PM_AE_UG_2_5);
    
    // PM 10 Small Number
    display.setTextSize(1);
    display.setCursor(10, 45);
    display.print("PM10:  ");
    display.print(data.PM_AE_UG_10_0);

    display.display(); 
  }
}