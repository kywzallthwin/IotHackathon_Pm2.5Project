#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PMS.h"
#include <WiFi.h>  // Added WiFi library

// --- WIFI CONFIGURATION ---
const char* wifiSsid = "SIT-HACKATHON";         // REPLACE with your Wi-Fi Name
const char* wifiPassword = "sithackathon"; // REPLACE with your Wi-Fi Password

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
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Screen failed to start - Check wires!"));
    for(;;); // Stop here if screen fails
  }
  
  // Show startup text for Wi-Fi
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Connecting to WiFi...");
  display.display();

  // 3. Connect to Wi-Fi
  WiFi.begin(wifiSsid, wifiPassword);
  
  int retry_count = 0;
  // Try connecting for about 10 seconds (20 * 500ms)
  while (WiFi.status() != WL_CONNECTED && retry_count < 20) {
    delay(500);
    Serial.print(".");
    display.print("."); // Add visual dots to screen
    display.display();
    retry_count++;
  }
  
  display.clearDisplay();
  display.setCursor(0, 10);

  // Check connection result
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    display.println("WiFi Connected!");
    display.print("IP: ");
    display.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi Connection Failed");
    display.println("WiFi Failed :(");
    display.println("Continuing offline...");
  }
  
  display.display();
  delay(2000); // Pause to let user read the status
  
  Serial.println("\n--- SYSTEM READY ---");
}

void loop() {
  if (pms.read(data)) {
    // --- PRINT TO SERIAL MONITOR ---
    Serial.print("PM 2.5: ");
    Serial.print(data.PM_AE_UG_2_5);
    Serial.print(" | WiFi Status: ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");

    // --- PRINT TO OLED SCREEN ---
    display.clearDisplay();
    
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Air Quality (ug/m3)");

    // --- WIFI INDICATOR ---
    // Draw a small filled circle in top right if connected
    if(WiFi.status() == WL_CONNECTED) {
        display.fillCircle(124, 3, 2, WHITE); 
    }
    
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