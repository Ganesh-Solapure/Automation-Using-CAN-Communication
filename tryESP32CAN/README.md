# IoT Sensor Network with CAN Bus Communication

This project implements a distributed IoT sensor network using ESP32 microcontrollers and CAN bus communication. The system consists of three main nodes: a sensor node, an actuator node, and a monitoring node, all communicating through a CAN bus network.

## Project Overview

The system is designed to create a robust and reliable sensor network that can:
- Monitor environmental conditions (temperature, humidity)
- Detect proximity using ultrasonic sensors
- Sense magnetic fields using Hall effect sensors
- Detect objects using IR sensors
- Control actuators based on sensor readings
- Monitor and display system status

## System Architecture

### Node1_Sensor
- Primary sensor node that collects various environmental and proximity data
- Sensors implemented:
  - DHT11 (Temperature and Humidity)
  - Ultrasonic Distance Sensor
  - IR Sensor
  - Hall Effect Sensor
- Communicates data via CAN bus at 1Mbps
- Implements error handling and retry mechanisms

### Node2_Actuator
- Controls actuators based on sensor data received from Node1
- Implements actuator control logic
- Responds to commands via CAN bus

### Node3_Monitor
- Monitors and displays system status
- Receives and processes data from both Node1 and Node2
- Provides system-wide monitoring capabilities

## Hardware Requirements

### Node1_Sensor
- ESP32 Development Board
- DHT11 Temperature and Humidity Sensor
- HC-SR04 Ultrasonic Sensor
- IR Sensor Module
- Hall Effect Sensor
- CAN Transceiver (e.g., MCP2515)

### Node2_Actuator
- ESP32 Development Board
- Actuators (type depends on application)
- CAN Transceiver

### Node3_Monitor
- ESP32 Development Board
- Display Module (if required)
- CAN Transceiver

## Software Requirements

- Arduino IDE
- Required Libraries:
  - CAN.h
  - DHT.h
  - Adafruit_Sensor.h
  - Ultrasonic.h

## Setup Instructions

1. **Hardware Setup**
   - Connect all sensors to Node1_Sensor according to pin definitions
   - Connect actuators to Node2_Actuator
   - Connect display (if used) to Node3_Monitor
   - Connect all nodes to the CAN bus network

2. **Software Setup**
   - Install Arduino IDE
   - Install required libraries through Arduino Library Manager
   - Open each node's .ino file in Arduino IDE
   - Configure the correct board settings for ESP32
   - Upload the code to respective nodes

3. **Configuration**
   - Adjust timing constants in the code if needed
   - Configure CAN bus speed (default: 1Mbps)
   - Set appropriate sensor update intervals

## Communication Protocol

The system uses CAN bus communication with the following specifications:
- Baud Rate: 1Mbps
- Message ID for Sensor Data: 0x100
- Data Structure:
  - Temperature (1 byte)
  - Humidity (1 byte)
  - Distance (2 bytes)
  - IR State (1 byte)
  - Hall State (1 byte)
  - Error Code (1 byte)
  - Reserved (1 byte)

## Error Handling

The system implements robust error handling:
- CAN communication retry mechanism
- Sensor reading validation
- Error code reporting
- Automatic reconnection attempts

## Debugging

Each node includes debug output through Serial Monitor:
- Sensor readings
- Communication status
- Error codes
- System state information

## Contributing

Feel free to contribute to this project by:
1. Forking the repository
2. Creating a feature branch
3. Submitting a pull request

## License

This project is open source and available under the MIT License.

## Contact

For any questions or support, please open an issue in the repository. 