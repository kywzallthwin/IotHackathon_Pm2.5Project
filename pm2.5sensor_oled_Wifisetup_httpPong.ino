#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PMS.h"
#include <WiFi.h>
#include <WebServer.h>

// --- WIFI CONFIGURATION ---
// press EN on board to get ip address at the start
const char* wifiSsid = "SIT-HACKATHON";     
const char* wifiPassword = "sithackathon"; 

// --- WEB SERVER CONFIGURATION ---
WebServer server(8010); 

// --- OLED CONFIGURATION ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- PMS SENSOR CONFIGURATION ---
PMS pms(Serial2);
PMS::DATA data;

// Global variables for web server
int currentPM25 = 0;
int currentPM10 = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize PMS Sensor
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Screen failed to start"));
    for(;;);
  }
  
  // Wi-Fi Connection UI
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.begin(wifiSsid, wifiPassword);
  
  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED && retry_count < 20) {
    delay(500);
    display.print(".");
    display.display();
    retry_count++;
  }
  
  display.clearDisplay();
  display.setCursor(0, 10);

  if(WiFi.status() == WL_CONNECTED) {
    // --- PRINT IP TO SERIAL MONITOR (Added Here) ---
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());  // <--- THIS PRINTS THE IP TO SERIAL
    Serial.println("Port: 8010");

    // --- Show on OLED ---
    display.println("WiFi Connected!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.print("Port: 8010"); 
    
    // --- SERVER SETUP ---
    server.on("/ping", HTTP_GET, []() {
      server.send(200, "text/plain", "pong");
    });

    server.on("/", HTTP_GET, []() {
      String message = "Air Quality Monitor\n\n";
      message += "PM 2.5: " + String(currentPM25) + " ug/m3\n";
      message += "PM 10:  " + String(currentPM10) + " ug/m3";
      server.send(200, "text/plain", message);
    });

    server.begin();
    Serial.println("HTTP Server started");

  } else {
    Serial.println("WiFi Failed");
    display.println("WiFi Failed :(");
  }
  
  display.display();
  delay(2000); 
}

void loop() {
  server.handleClient();

  if (pms.read(data)) {
    currentPM25 = data.PM_AE_UG_2_5;
    currentPM10 = data.PM_AE_UG_10_0;

    // --- PRINT TO SERIAL ---
    Serial.print("PM 2.5: ");
    Serial.println(currentPM25);

    // --- PRINT TO OLED ---
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Air Quality (ug/m3)");

    if(WiFi.status() == WL_CONNECTED) {
        display.fillCircle(124, 3, 2, WHITE); 
    }
    
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print("PM2.5: ");
    display.print(currentPM25);
    
    display.setTextSize(1);
    display.setCursor(10, 45);
    display.print("PM10:  ");
    display.print(currentPM10);

    display.display(); 
  }
}