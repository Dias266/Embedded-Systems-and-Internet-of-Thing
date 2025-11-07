# Setup Instructions

## Prerequisites
- Arduino IDE installed
- Node.js and npm installed
- MQTT broker running (Mosquitto recommended)

## Hardware Setup

### ESP32 Wiring:
1. DS18B20 Temperature Sensor:
   - VCC → 3.3V
   - GND → GND
   - DATA → GPIO 4 (with 4.7kΩ pull-up to 3.3V)

2. Status LEDs:
   - Green LED → GPIO 2 (with 220Ω resistor)
   - Yellow LED → GPIO 5 (with 220Ω resistor)
   - Red LED → GPIO 18 (with 220Ω resistor)

### Arduino Wiring:
1. LCD Display (16x2):
   - VSS → GND
   - VDD → 5V
   - V0 → Potentiometer wiper (10kΩ)
   - RS → Pin 12
   - RW → GND
   - EN → Pin 11
   - D4 → Pin 5
   - D5 → Pin 4
   - D6 → Pin 3
   - D7 → Pin 2
   - A → 5V (with 220Ω resistor)
   - K → GND

2. Status LEDs:
   - Green → Pin 8 (with 220Ω resistor)
   - Red → Pin 9 (with 220Ω resistor)
   - Yellow → Pin 10 (with 220Ω resistor)

## Software Setup

### Step 1: MQTT Broker
```bash
# Install Mosquitto (Ubuntu/Debian)
sudo apt-get install mosquitto mosquitto-clients

# Start broker
sudo systemctl start mosquitto

# Test broker
mosquitto_sub -t "test" -v
```

### Step 2: ESP32 Setup
1. Open `esp32_telematics.ino` in Arduino IDE
2. Create `config.h` from `config.h.example`
3. Update WiFi credentials in `config.h`
4. Update MQTT broker IP in `config.h`
5. Install required libraries (see libraries_required.txt)
6. Select Board: ESP32 Dev Module
7. Upload to ESP32

### Step 3: Arduino Setup
1. Open `arduino_auth_controller.ino` in Arduino IDE
2. Create `arduino_config.h` from example
3. Install required libraries
4. Select Board: Arduino Uno
5. Upload to Arduino

### Step 4: Control Unit Setup
```bash
# Install dependencies
npm install

# Copy config
cp config.example.js config.js

# Edit config.js with your settings
nano config.js

# Update serial port (find with: ls /dev/tty*)
# Linux: /dev/ttyUSB0 or /dev/ttyACM0
# Mac: /dev/cu.usbserial*
# Windows: COM3, COM4, etc.

# Run control unit
node control-unit-iot.js
```

### Step 5: Dashboard
```bash
# Open dashboard-iot.html in browser
# Or serve it:
python -m http.server 8080
# Then open: http://localhost:8080/dashboard-iot.html
```

## Troubleshooting

### ESP32 Not Connecting to WiFi:
- Check SSID/password in config.h
- Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check serial monitor for error messages

### Arduino Serial Communication Failing:
- Verify correct serial port in config.js
- Check baud rate matches (9600)
- Ensure Arduino is powered

### MQTT Messages Not Received:
- Check MQTT broker is running: `sudo systemctl status mosquitto`
- Test with: `mosquitto_sub -t "vehicle/#" -v`
- Verify MQTT broker IP/port in ESP32 config

### Dashboard Not Updating:
- Check browser console for errors
- Verify control unit is running
- Ensure WebSocket connection established
