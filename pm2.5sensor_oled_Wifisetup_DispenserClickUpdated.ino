#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PMS.h"
#include <WiFi.h>
#include <WebServer.h>

// --- WIFI CONFIGURATION ---
const char* wifiSsid = "SIT-HACKATHON";     
const char* wifiPassword = "sithackathon"; 

// --- CONFIGURATION ---
const int MOTOR_PIN = 4;
const int DANGER_THRESHOLD = 50; // Triggers ALERT if PM2.5 is higher than this

// --- OLED CONFIGURATION ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- PMS SENSOR ---
PMS pms(Serial2);
PMS::DATA data;

// --- SERVER ---
WebServer server(8010); 

// Global Variables
int currentPM25 = 0;
int currentPM10 = 0;
String statusMsg = "Ready";

// --- HTML PAGE DESIGN ---
String getHTML() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Mask Dispenser</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".sensor {font-size: 20px; color: #888; margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Smart Mask Dispenser</h1>\n";
  
  // Display Sensor Data
  ptr +="<div class='sensor'>PM 2.5: <b>" + String(currentPM25) + "</b> ug/m3</div>\n";
  ptr +="<div class='sensor'>PM 10: <b>" + String(currentPM10) + "</b> ug/m3</div>\n";
  ptr +="<p>Status: " + statusMsg + "</p>\n";

  // Display Buttons
  ptr +="<a class=\"button button-on\" href=\"/dispense\">DISPENSE MASK</a>\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW); // Motor OFF initially

  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Screen failed"));
    for(;;);
  }
  
  // Wi-Fi Connect
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(wifiSsid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());
  
  // Show IP briefly at startup so you know where to connect
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("WiFi Connected!");
  display.println(WiFi.localIP());
  display.display();
  delay(3000); // Hold for 3 seconds so user can read it

  // --- WEB SERVER ENDPOINTS ---
  
  // 1. Root Page (The Dashboard)
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getHTML());
  });

  // 2. Dispense Action
  server.on("/dispense", HTTP_GET, []() {
    statusMsg = "Dispensing...";
    server.send(200, "text/html", getHTML()); 
    
    // Motor Action
    Serial.println("Dispensing Mask...");
    digitalWrite(MOTOR_PIN, HIGH); 
    
    // Override Display for Dispensing Status
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(BLACK, WHITE); // Invert colors
    display.setCursor(5, 25);
    display.println("DISPENSING");
    display.display();
    display.setTextColor(WHITE); // Reset colors
    
    delay(1000); // Run for 1 second
    
    digitalWrite(MOTOR_PIN, LOW);  
    statusMsg = "Ready";
    Serial.println("Dispense Complete");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (pms.read(data)) {
    currentPM25 = data.PM_AE_UG_2_5;
    currentPM10 = data.PM_AE_UG_10_0;
    
    // --- CLEAN OLED DISPLAY LOGIC ---
    display.clearDisplay();
    
    // 1. Label
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("PM 2.5 Level");

    // 2. The Big Number
    display.setTextSize(3); // Huge font
    display.setCursor(0, 15);
    display.print(currentPM25);
    display.setTextSize(1);
    display.print(" ug/m3");

    // 3. Dangerous Alert State
    if (currentPM25 > DANGER_THRESHOLD) {
      // DANGER MODE: Inverted Text background
      display.fillRect(0, 45, 128, 19, WHITE); // White box at bottom
      display.setTextColor(BLACK); // Black text
      display.setTextSize(2);
      display.setCursor(20, 47);
      display.print("DANGER!");
      
      // Optional: Add small buzzer beep here later if needed
    } else {
      // SAFE MODE: Normal Text
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.setCursor(0, 47);
      display.print("Air: GOOD");
    }

    display.display();
    
    // Reset text color for next loop
    display.setTextColor(WHITE);
  }
}