// config.example.js
module.exports = {
    // WiFi Configuration (for ESP32)
    WIFI_SSID: "your_wifi_network",
    WIFI_PASSWORD: "your_wifi_password",
    
    // MQTT Broker Configuration
    MQTT_BROKER: "localhost",  // or "test.mosquitto.org"
    MQTT_PORT: 1883,
    MQTT_TOPIC_TELEMETRY: "vehicle/telemetry",
    MQTT_TOPIC_STATE: "vehicle/state",
    
    // Serial Port Configuration
    SERIAL_PORT: "/dev/ttyUSB0",  // Arduino port
    SERIAL_BAUDRATE: 9600,
    
    // Test Vehicle Configuration
    TEST_VIN: "1HGCM82633A123456",
    
    // System Configuration
    DASHBOARD_PORT: 3000,
    TEMPERATURE_THRESHOLDS: {
        NORMAL: 30,
        WARNING: 40
    }
};
