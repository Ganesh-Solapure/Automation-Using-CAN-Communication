#include <CAN.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include "Ultrasonic.h"

// Pin Definitions
#define DHT_PIN 4
#define DHT_TYPE DHT11
#define TRIG_PIN 5
#define ECHO_PIN 6
#define IR_PIN 13
#define HALL_PIN 8

// CAN Message IDs
#define SENSOR_DATA_ID 0x100

// Timing Constants
#define SENSOR_UPDATE_INTERVAL 2000  // 2 seconds for DHT (DHT11 needs 2s between readings)
#define CAN_RETRY_INTERVAL 100       // 100ms for CAN retry
#define MAX_RETRIES 3
#define CAN_INIT_RETRY_DELAY 1000    // 1 second between CAN init retries

// Sensor Objects
DHT dht(DHT_PIN, DHT_TYPE);
Ultrasonic ultrasonic(TRIG_PIN, ECHO_PIN);

// CAN Message Structure
struct SensorData {
  uint8_t temperature;
  uint8_t humidity;
  uint16_t distance;
  uint8_t irState;
  uint8_t hallState;
  uint8_t errorCode;
  uint8_t reserved;
} sensorData;

// Timing variables
unsigned long lastSensorUpdate = 0;
unsigned long lastCanRetry = 0;
uint8_t retryCount = 0;
bool canInitialized = false;
unsigned long lastCANInitAttempt = 0;

void setup() {
  Serial.begin(115200);
  delay(100); // Allow serial port to settle
  Serial.println("\nNode1_Sensor: Initializing...");
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize pins
  pinMode(IR_PIN, INPUT);
  pinMode(HALL_PIN, INPUT);
  
  // Initialize sensor data
  memset(&sensorData, 0, sizeof(sensorData));
  
  // Initialize CAN with retry mechanism
  initializeCAN();
  
  Serial.println("Node1_Sensor: Setup complete");
}

void initializeCAN() {
  Serial.println("Initializing CAN...");
  if (CAN.begin(1000E3)) {  // 1Mbps
    Serial.println("CAN initialized successfully");
    CAN.filter(SENSOR_DATA_ID);
    canInitialized = true;
  } else {
    Serial.println("CAN initialization failed");
    canInitialized = false;
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Attempt CAN re-initialization periodically without blocking
  if (!canInitialized && (currentMillis - lastCANInitAttempt >= CAN_INIT_RETRY_DELAY)) {
    initializeCAN();
    lastCANInitAttempt = currentMillis;
  }
  
  // Update sensors at regular interval
  if (currentMillis - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL) {
    updateSensors();
    lastSensorUpdate = currentMillis;
    retryCount = 0;  // Reset retry counter on successful update
  }
  
  // Send CAN message with retry mechanism
  if (currentMillis - lastCanRetry >= CAN_RETRY_INTERVAL) {
    if (!sendCanMessage() && retryCount < MAX_RETRIES) {
      retryCount++;
      lastCanRetry = currentMillis;
    }
  }
}

void updateSensors() {
  // Read DHT sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Check if any reads failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    sensorData.errorCode |= 0x01;  // Set DHT error bit
  } else {
    sensorData.humidity = (uint8_t)humidity;
    sensorData.temperature = (uint8_t)temperature;
    sensorData.errorCode &= ~0x01;  // Clear DHT error bit
  }
  
  // Read Ultrasonic
  sensorData.distance = ultrasonic.read();
  if (sensorData.distance == 0) {
    Serial.println("Ultrasonic read failed!");
  }
  
  // Read IR Sensor
  sensorData.irState = digitalRead(IR_PIN);
  
  // Read Hall Sensor
  sensorData.hallState = digitalRead(HALL_PIN);
  
  // Print detailed sensor values for debugging
  Serial.println("\n--- Sensor Readings ---");
  Serial.print("Temperature: "); 
  Serial.print(sensorData.temperature);
  Serial.println("°C");
  
  Serial.print("Humidity: "); 
  Serial.print(sensorData.humidity);
  Serial.println("%");
  
  Serial.print("Distance: "); 
  Serial.print(sensorData.distance);
  Serial.println("cm");
  
  Serial.print("IR Sensor: "); 
  Serial.println(sensorData.irState ? "ON" : "OFF");
  
  Serial.print("Hall Sensor: "); 
  Serial.println(sensorData.hallState ? "ON" : "OFF");
  
  Serial.print("Error Code: 0x");
  if (sensorData.errorCode < 16) Serial.print("0");
  Serial.println(sensorData.errorCode, HEX);
  Serial.println("-------------------\n");
}

bool sendCanMessage() {
  if (!canInitialized) {
    Serial.println("CAN not initialized, attempting to reinitialize...");
    initializeCAN();
    return false;
  }

  CAN.beginPacket(SENSOR_DATA_ID);
  CAN.write((uint8_t*)&sensorData, sizeof(sensorData));
  
  if (CAN.endPacket()) {
    Serial.println("CAN message sent successfully");
    sensorData.errorCode &= ~0x02;  // Clear CAN transmission error bit
    return true;
  } else {
    Serial.println("CAN message transmission failed");
    sensorData.errorCode |= 0x02;  // Set CAN transmission error bit
    return false;
  }
} 
