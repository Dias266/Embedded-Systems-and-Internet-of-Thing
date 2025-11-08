# VIN-Based Telematics Authentication System

**Secure IoT Vehicle Data Monitoring with Blockchain Integration**

University of Bologna - Embedded Systems & IoT Course Project

---

## Overview

This project implements a **4-subsystem IoT architecture** for secure vehicle telematics using VIN-based cryptographic authentication:

- **ESP32 Telematics Device** - Collects temperature, OBD-II diagnostics, and signs data
- **Arduino Authentication Controller** - Verifies ECDSA signatures via hardware
- **Node.js Control Unit** - Orchestrates MQTT, Serial, HTTP communication
- **Web Dashboard** - Real-time visualization of telemetry data

Authenticated data is forwarded to **Hyperledger Fabric blockchain** for immutable storage and exposed via API for machine learning analysis.

**Key Features:**
- FSM-based adaptive sampling (NORMAL → WARNING → CRITICAL)
- VIN-derived cryptographic signatures
- Multi-protocol communication (MQTT, Serial, HTTP)
- Real-time web dashboard with live graphs
- Blockchain integration for distributed systems

---

## Hardware Requirements

### ESP32 Subsystem
- ESP32 DevKit V1
- DS18B20 temperature sensor
- 3x LEDs (Green, Yellow, Red)
- 3x 220Ω resistors + 1x 4.7kΩ pull-up resistor
- Breadboard and jumper wires

### Arduino Subsystem
- Arduino Uno R3
- 16x2 LCD display (HD44780)
- 3x LEDs (Green, Red, Yellow)
- 3x 220Ω resistors + 1x 10kΩ potentiometer
- USB cable
- Breadboard and jumper wires

---

## Software Requirements

- **Arduino IDE** 2.0+
- **Node.js** 18.x+
- **Mosquitto MQTT Broker**

**Arduino Libraries:**
- WiFi (ESP32 built-in)
- PubSubClient (MQTT)
- DallasTemperature (DS18B20)
- OneWire
- LiquidCrystal (built-in)

**Node.js Packages:**
```bash
npm install mqtt serialport @serialport/parser-readline express cors
```

---

## Quick Start

### 1. Install Dependencies

```bash
# Clone repository
git clone https://github.com/Dias266/Embedded-Systems-and-Internet-of-Thing.git
cd Embedded-Systems-and-Internet-of-Thing

# Install Node.js dependencies
npm install

# Install MQTT broker
# Linux: sudo apt-get install mosquitto
# Mac: brew install mosquitto
# Windows: Download from mosquitto.org
```

### 2. Configure System

**Update `config.h` for ESP32:**
```cpp
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourPassword"
#define MQTT_BROKER "192.168.1.XXX"  // Your computer's IP address
```

**Find your computer's IP:**
- Windows: `ipconfig`
- Mac/Linux: `ifconfig`

**Update `control-unit-iot.js` line 13:**
```javascript
const SERIAL_PORT = '/dev/ttyUSB0';  // Change to your Arduino port
```

**Find Arduino port:**
- Linux: `ls /dev/tty* | grep -E 'USB|ACM'`
- Mac: `ls /dev/cu.*`
- Windows: Arduino IDE → Tools → Port

### 3. Upload Code

**Arduino:**
1. Open `arduino_auth_controller.ino` in Arduino IDE
2. Select Board: Arduino Uno
3. Select Port: Your Arduino port
4. Click Upload

**ESP32:**
1. Open `esp32_telematics.ino` in Arduino IDE
2. Select Board: ESP32 Dev Module
3. Select Port: Your ESP32 port
4. Click Upload

### 4. Run System

```bash
# Terminal 1: Start MQTT broker
sudo systemctl start mosquitto  # Linux
# or brew services start mosquitto  # Mac

# Terminal 2: Start Control Unit
node control-unit-iot.js

# Terminal 3: Open Dashboard
# Open dashboard-iot.html in web browser
```

**Expected Output:**
```
MQTT Connected
Serial connection established with Arduino
HTTP Server: http://localhost:3002
Monitoring vehicle telemetry...
```

---

## System Architecture

```
ESP32 (Telematics) --MQTT--> Control Unit --HTTP--> Dashboard
                                  |
                               Serial
                                  |
                          Arduino (Auth)
```

**Data Flow:**
1. ESP32 collects temperature, signs with VIN-based signature
2. ESP32 publishes to MQTT broker
3. Control Unit receives via MQTT, forwards to Arduino via Serial
4. Arduino verifies signature, sends result back
5. Control Unit stores authenticated data
6. Dashboard fetches data via HTTP API and displays in real-time

---

## Troubleshooting

### LCD Shows Nothing
- Adjust the 10kΩ potentiometer slowly until text appears
- Verify Pin 5 (RW) of LCD is connected to GND

### MQTT Connection Failed (rc=-2)
- Update `config.h` with your **actual computer's IP address**
- Verify Mosquitto is running: `sudo systemctl status mosquitto`
- Check firewall allows port 1883

### Arduino Not Found
- Update `control-unit-iot.js` with correct serial port
- Close Arduino IDE Serial Monitor before running control unit
- Linux: Add user to dialout group: `sudo usermod -a -G dialout $USER`

### Temperature Sensor Error
- Add 4.7kΩ pull-up resistor between DS18B20 DATA and 3.3V
- For testing without sensor, code uses default 25.0°C

---

## API Endpoints

```
GET  /api/telemetry/latest      - Latest telemetry data
GET  /api/telemetry/history     - Historical data (last 100)
GET  /api/telemetry/statistics  - Temperature stats
GET  /api/status                - System health check
```

---

## Testing

```bash
# Test MQTT connectivity
node test-mqtt.js

# Test Serial communication
node test-serial.js
```

---

## Project Structure

```
├── esp32_telematics.ino          # ESP32 telematics code
├── arduino_auth_controller.ino   # Arduino authentication code
├── control-unit-iot.js           # Node.js backend
├── dashboard-iot.html            # Web dashboard
├── config.h                      # ESP32 configuration
├── test-mqtt.js                  # MQTT test script
├── test-serial.js                # Serial test script
└── package.json                  # Node.js dependencies
```

---

## Integration

### Blockchain (Hyperledger Fabric)
Authenticated telemetry is forwarded to Hyperledger Fabric for:
- Immutable storage
- Byzantine fault tolerance
- Distributed consensus

### Machine Learning
Data exported via API for:
- Anomaly detection
- Digital twin modeling
- Predictive maintenance

---

## Video Demonstration

[Link to Demo Video](https://youtu.be/your-link)

---

## Contributors

**Dias Katrenov** - ESP32 Development, System Integration  
**Mary Anne Selirio** - Arduino Development, Dashboard, Documentation

**University of Bologna**  
Embedded Systems and IoT Course  
Professors: Ricci, Omicini, Golfarelli/Francia

---

## License

MIT License - See LICENSE file for details

---

## Repository

**GitHub:** [https://github.com/Dias266/Embedded-Systems-and-Internet-of-Thing](https://github.com/Dias266/Embedded-Systems-and-Internet-of-Thing)

---

**⭐ Star this repo if you find it useful!**
