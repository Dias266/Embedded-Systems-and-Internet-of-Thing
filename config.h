// config.h
#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// MQTT Broker Configuration
#define MQTT_BROKER "192.168.1.100"  // Replace with your MQTT broker IP
#define MQTT_PORT 1883
#define MQTT_TOPIC_TELEMETRY "vehicle/telemetry"
#define MQTT_TOPIC_STATE "vehicle/state"

// Temperature Sensor Configuration
#define ONE_WIRE_BUS 4  // GPIO4 for DS18B20
#define TEMP_PRECISION 12

// LED Pin Configuration
#define LED_GREEN 2
#define LED_YELLOW 5
#define LED_RED 18

// Temperature Thresholds
#define TEMP_NORMAL 30.0
#define TEMP_WARNING 40.0

// Sampling Frequencies (milliseconds)
#define FREQ_NORMAL 5000   // 5 seconds
#define FREQ_WARNING 2000  // 2 seconds
#define FREQ_CRITICAL 1000 // 1 second

// Test Vehicle VIN
#define TEST_VIN "1HGCM82633A123456"

#endif
