#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PMS.h"
#include <WiFi.h>
#include <WebServer.h>

// --- WIFI CONFIGURATION ---
const char* wifiSsid = "SIT-HACKATHON";     
const char* wifiPassword = "sithackathon"; 

// --- HARDWARE CONFIGURATION ---
const int MOTOR_PIN = 4;
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
// This makes the web page look nice with buttons
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

  // --- WEB SERVER ENDPOINTS ---
  
  // 1. Root Page (The Dashboard)
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getHTML());
  });

  // 2. Dispense Action (Run for 1 second then stop)
  server.on("/dispense", HTTP_GET, []() {
    statusMsg = "Dispensing...";
    server.send(200, "text/html", getHTML()); // Refresh page immediately
    
    // Motor Action
    Serial.println("Dispensing Mask...");
    digitalWrite(MOTOR_PIN, HIGH); // Motor ON
    
    // Update OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.println("DISPENSING");
    display.display();
    
    delay(1000); // Run for 1 second (Adjust this number if needed!)
    
    digitalWrite(MOTOR_PIN, LOW);  // Motor OFF
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
    
    // Normal OLED Display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("IP: "); 
    display.println(WiFi.localIP());
    
    display.setCursor(0, 15);
    display.println("---------------------");

    display.setTextSize(2);
    display.setCursor(10, 30);
    display.print("PM2.5: ");
    display.print(currentPM25);
    display.display();
  }
}