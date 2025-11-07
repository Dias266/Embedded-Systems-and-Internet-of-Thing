// config.h - ESP32 Configuration
// YOU MUST UPDATE THIS FILE WITH YOUR WIFI AND IP ADDRESS!

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration - CHANGE THESE!

#define WIFI_SSID "YOUR_WIFI_NAME"        // ← Put your WiFi network name here
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD" // ← Put your WiFi password here

// MQTT Broker Configuration - CHANGE THIS!

// This should be your computer's IP address (not "localhost"!)
// To find your IP:
//   Windows: ipconfig
//   Mac/Linux: ifconfig
//   Look for something like: 192.168.1.100 or 10.0.0.5
#define MQTT_BROKER "192.168.1.100"  // ← Put your computer's IP here!
#define MQTT_PORT 1883
#define MQTT_TOPIC_TELEMETRY "vehicle/telemetry"
#define MQTT_TOPIC_STATE "vehicle/state"

// Hardware Pin Configuration - Don't change unless wiring is different

#define ONE_WIRE_BUS 4  // GPIO4 for DS18B20 temperature sensor
#define TEMP_PRECISION 12

// LED Pins
#define LED_GREEN 2
#define LED_YELLOW 5
#define LED_RED 18

// Temperature Thresholds - Can adjust if needed
#define TEMP_NORMAL 30.0    // Below this = NORMAL state
#define TEMP_WARNING 40.0   // Above this = CRITICAL state

// Sampling Frequencies (milliseconds) - Can adjust if needed
#define FREQ_NORMAL 5000    // 5 seconds in NORMAL state
#define FREQ_WARNING 2000   // 2 seconds in WARNING state
#define FREQ_CRITICAL 1000  // 1 second in CRITICAL state

// Test Vehicle VIN - Can use any VIN for testing
#define TEST_VIN "1HGCM82633A123456"

#endif
