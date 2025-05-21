#include <CAN.h>

// Pin Definitions
#define RELAY_PIN 4
#define BUZZER_PIN 5
#define LED_PIN 6
#define MOTOR_PIN 7
#define ERROR_LED_PIN 8
#define STATUS_LED_PIN 9

// CAN Message IDs
#define SENSOR_DATA_ID 0x100
#define ACTUATOR_STATUS_ID 0x200

// Timing Constants
#define STATUS_UPDATE_INTERVAL 500   // 500ms for status updates
#define ERROR_BLINK_INTERVAL 200     // 200ms for error LED blink
#define STATUS_BLINK_INTERVAL 100    // 100ms for status LED blink

// Thresholds
#define TEMP_THRESHOLD 30
#define DISTANCE_THRESHOLD 50

// CAN Message Structure
struct ActuatorStatus {
  uint8_t relayState;
  uint8_t buzzerState;
  uint8_t ledState;
  uint8_t motorSpeed;
  uint8_t errorCode;
  uint8_t reserved[3];
} actuatorStatus;

// Timing variables
unsigned long lastStatusUpdate = 0;
unsigned long lastErrorBlink = 0;
unsigned long lastStatusBlink = 0;
bool errorLedState = false;
bool statusLedState = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize CAN with higher speed
  if (!CAN.begin(1000E3)) {  // 1Mbps for faster communication
    Serial.println("Starting CAN failed!");
    while (1);
  }
  
  // Configure CAN filter for sensor data
  CAN.filter(SENSOR_DATA_ID);
  
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(ERROR_LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Initialize PWM for motor
  ledcSetup(0, 5000, 8);
  ledcAttachPin(MOTOR_PIN, 0);
  
  // Initialize actuator status
  memset(&actuatorStatus, 0, sizeof(actuatorStatus));
  
  Serial.println("Actuator Node Initialized");
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
      
      // Process sensor data and control actuators
      processSensorData(data);
      
      // Update status LED
      statusLedState = true;
      lastStatusBlink = currentMillis;
    }
  }
  
  // Update status message periodically
  if (currentMillis - lastStatusUpdate >= STATUS_UPDATE_INTERVAL) {
    sendActuatorStatus();
    lastStatusUpdate = currentMillis;
  }
  
  // Handle error LED blinking
  if (actuatorStatus.errorCode != 0) {
    if (currentMillis - lastErrorBlink >= ERROR_BLINK_INTERVAL) {
      errorLedState = !errorLedState;
      digitalWrite(ERROR_LED_PIN, errorLedState);
      lastErrorBlink = currentMillis;
    }
  } else {
    digitalWrite(ERROR_LED_PIN, LOW);
  }
  
  // Handle status LED blinking
  if (statusLedState) {
    if (currentMillis - lastStatusBlink >= STATUS_BLINK_INTERVAL) {
      statusLedState = false;
      digitalWrite(STATUS_LED_PIN, LOW);
    } else {
      digitalWrite(STATUS_LED_PIN, HIGH);
    }
  }
}

void processSensorData(uint8_t* data) {
  uint8_t temperature = data[0];
  uint8_t humidity = data[1];
  uint16_t distance = (data[2] << 8) | data[3];
  uint8_t irState = data[4];
  uint8_t hallState = data[5];
  uint8_t errorCode = data[6];
  
  // Update error code
  actuatorStatus.errorCode = errorCode;
  
  // Basic threshold logic
  if (temperature > TEMP_THRESHOLD) {
    digitalWrite(RELAY_PIN, HIGH);
    actuatorStatus.relayState = 1;
  } else {
    digitalWrite(RELAY_PIN, LOW);
    actuatorStatus.relayState = 0;
  }
  
  if (distance < DISTANCE_THRESHOLD) {
    digitalWrite(BUZZER_PIN, HIGH);
    actuatorStatus.buzzerState = 1;
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    actuatorStatus.buzzerState = 0;
  }
  
  // Control LED based on IR sensor
  digitalWrite(LED_PIN, irState);
  actuatorStatus.ledState = irState;
  
  // Control motor speed based on hall sensor
  uint8_t motorSpeed = hallState ? 255 : 0;
  ledcWrite(0, motorSpeed);
  actuatorStatus.motorSpeed = motorSpeed;
  
  // Print actuator states for debugging
  Serial.print("Relay: "); Serial.print(actuatorStatus.relayState);
  Serial.print(", Buzzer: "); Serial.print(actuatorStatus.buzzerState);
  Serial.print(", LED: "); Serial.print(actuatorStatus.ledState);
  Serial.print(", Motor: "); Serial.println(actuatorStatus.motorSpeed);
}

void sendActuatorStatus() {
  CAN.beginPacket(ACTUATOR_STATUS_ID);
  CAN.write((uint8_t*)&actuatorStatus, sizeof(actuatorStatus));
  if (CAN.endPacket()) {
    Serial.println("Status sent");
  } else {
    Serial.println("Status failed");
    actuatorStatus.errorCode |= 0x01;  // Set CAN error bit
  }
} 