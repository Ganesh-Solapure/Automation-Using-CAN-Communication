#include <CAN.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// CAN Message IDs
#define SENSOR_DATA_ID 0x100
#define ACTUATOR_STATUS_ID 0x200

// LCD Configuration
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Timing Constants
#define DISPLAY_UPDATE_INTERVAL 500   // 500ms for display updates
#define ERROR_DISPLAY_TIME 2000       // 2 seconds for error display
#define SCROLL_INTERVAL 1000          // 1 second for scrolling

// Create LCD object
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Display update interval
unsigned long lastDisplayUpdate = 0;
unsigned long lastScrollUpdate = 0;
bool showError = false;
unsigned long errorStartTime = 0;
uint8_t currentPage = 0;

// Sensor data storage
struct SensorData {
  uint8_t temperature;
  uint8_t humidity;
  uint16_t distance;
  uint8_t irState;
  uint8_t hallState;
  uint8_t errorCode;
} sensorData;

void setup() {
  Serial.begin(115200);
  
  // Initialize CAN with higher speed
  if (!CAN.begin(1000E3)) {  // 1Mbps for faster communication
    Serial.println("Starting CAN failed!");
    while (1);
  }
  
  // Configure CAN filters for both sensor and actuator data
  CAN.filter(SENSOR_DATA_ID);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Initialize sensor data
  memset(&sensorData, 0, sizeof(sensorData));
  
  // Display initial message
  lcd.setCursor(0, 0);
  lcd.print("CAN Monitor Node");
  lcd.setCursor(0, 1);
  lcd.print("Waiting for data...");
  
  Serial.println("Monitor Node Initialized");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check for incoming CAN messages
  int packetSize = CAN.parsePacket();
  
  if (packetSize) {
    if (CAN.packetId() == SENSOR_DATA_ID) {
      uint8_t data[8];
      int i = 0;
      while (CAN.available() && i < 8) {
        data[i++] = CAN.read();
      }
      
      // Update sensor data
      updateSensorData(data);
      
      // Check for errors
      if (sensorData.errorCode != 0) {
        showError = true;
        errorStartTime = currentMillis;
      }
    }
  }
  
  // Update display at regular intervals
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    if (showError && (currentMillis - errorStartTime < ERROR_DISPLAY_TIME)) {
      displayError();
    } else {
      showError = false;
      displaySensorData();
    }
    lastDisplayUpdate = currentMillis;
  }
  
  // Handle page scrolling
  if (currentMillis - lastScrollUpdate >= SCROLL_INTERVAL) {
    currentPage = (currentPage + 1) % 2;  // Toggle between two pages
    lastScrollUpdate = currentMillis;
  }
}

void updateSensorData(uint8_t* data) {
  sensorData.temperature = data[0];
  sensorData.humidity = data[1];
  sensorData.distance = (data[2] << 8) | data[3];
  sensorData.irState = data[4];
  sensorData.hallState = data[5];
  sensorData.errorCode = data[6];
  
  // Print received data for debugging
  Serial.print("Temp: "); Serial.print(sensorData.temperature);
  Serial.print("°C, Hum: "); Serial.print(sensorData.humidity);
  Serial.print("%, Dist: "); Serial.print(sensorData.distance);
  Serial.print("cm, IR: "); Serial.print(sensorData.irState);
  Serial.print(", Hall: "); Serial.println(sensorData.hallState);
}

void displaySensorData() {
  lcd.clear();
  
  if (currentPage == 0) {
    // First page: Temperature and Humidity
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(sensorData.temperature);
    lcd.print("C");
    
    lcd.setCursor(0, 1);
    lcd.print("Hum:  ");
    lcd.print(sensorData.humidity);
    lcd.print("%");
  } else {
    // Second page: Distance and Sensors
    lcd.setCursor(0, 0);
    lcd.print("Dist: ");
    lcd.print(sensorData.distance);
    lcd.print("cm");
    
    lcd.setCursor(0, 1);
    lcd.print("IR:");
    lcd.print(sensorData.irState ? "ON " : "OFF");
    lcd.print(" H:");
    lcd.print(sensorData.hallState ? "ON" : "OFF");
  }
}

void displayError() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error Code:");
  lcd.setCursor(0, 1);
  lcd.print("0x");
  if (sensorData.errorCode < 16) lcd.print("0");
  lcd.print(sensorData.errorCode, HEX);
} 