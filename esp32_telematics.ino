/*
 * ESP32 Telematics Device - Vehicle Data Collection & Transmission
 * Collects: Temperature, OBD-II data, Mileage
 * Signs data with VIN-based ECDSA private key
 * Transmits via MQTT to Control Unit
 * 
 * FSM States: NORMAL → WARNING → CRITICAL (based on temperature)
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT Configuration
const char* mqtt_server = "localhost"; // Change to your Control Unit IP
const int mqtt_port = 1883;
const char* mqtt_topic_telemetry = "vehicle/telemetry";
const char* mqtt_topic_state = "vehicle/state";

// VIN Configuration (for signing)
const char* VEHICLE_VIN = "1HGCM82633A123456";

// Temperature sensor
#define ONE_WIRE_BUS 4
#define LED_NORMAL 2
#define LED_WARNING 5
#define LED_CRITICAL 18

// Temperature thresholds
#define TEMP_THRESHOLD_WARNING 30.0
#define TEMP_THRESHOLD_CRITICAL 40.0

// Sampling frequencies (milliseconds)
#define FREQUENCY_NORMAL 5000     // 5 seconds
#define FREQUENCY_WARNING 2000    // 2 seconds
#define FREQUENCY_CRITICAL 1000   // 1 second

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

WiFiClient espClient;
PubSubClient mqttClient(espClient);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// FSM States
enum SystemState {
  STATE_NORMAL,
  STATE_WARNING,
  STATE_CRITICAL
};

SystemState currentState = STATE_NORMAL;
unsigned long lastSampleTime = 0;
unsigned long samplingFrequency = FREQUENCY_NORMAL;

// Vehicle data
float currentTemperature = 0.0;
unsigned long currentMileage = 45000; // Simulated
String diagnosticCodes = "";

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  
  // Initialize LEDs
  pinMode(LED_NORMAL, OUTPUT);
  pinMode(LED_WARNING, OUTPUT);
  pinMode(LED_CRITICAL, OUTPUT);
  
  // Initialize temperature sensor
  sensors.begin();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Connect to MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  connectToMQTT();
  
  Serial.println("✅ ESP32 Telematics Device Initialized");
  Serial.println("VIN: " + String(VEHICLE_VIN));
  
  updateLEDs();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();
  
  // Check if it's time to sample
  unsigned long currentTime = millis();
  if (currentTime - lastSampleTime >= samplingFrequency) {
    lastSampleTime = currentTime;
    
    // Collect telemetry data
    collectTelemetryData();
    
    // Update FSM state based on temperature
    updateSystemState();
    
    // Sign and transmit data
    String telemetryData = buildTelemetryPacket();
    String signature = signData(telemetryData);
    
    String signedPacket = telemetryData + "|SIG:" + signature;
    
    // Publish to MQTT
    mqttClient.publish(mqtt_topic_telemetry, signedPacket.c_str());
    
    Serial.println("📡 Telemetry sent: " + telemetryData);
    Serial.println("🔐 Signature: " + signature.substring(0, 16) + "...");
  }
}

// ============================================================================
// WIFI CONNECTION
// ============================================================================

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected");
    Serial.println("IP: " + WiFi.localIP().toString());
    digitalWrite(LED_NORMAL, HIGH);
  } else {
    Serial.println("\n❌ WiFi connection failed");
    digitalWrite(LED_CRITICAL, HIGH);
  }
}

// ============================================================================
// MQTT CONNECTION
// ============================================================================

void connectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    
    String clientId = "ESP32_" + String(VEHICLE_VIN);
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("✅ MQTT connected");
      mqttClient.subscribe(mqtt_topic_state);
    } else {
      Serial.print("❌ MQTT failed, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("📥 MQTT received: " + message);
  
  // Handle state updates from Control Unit
  if (message == "NORMAL") {
    currentState = STATE_NORMAL;
    samplingFrequency = FREQUENCY_NORMAL;
  } else if (message == "WARNING") {
    currentState = STATE_WARNING;
    samplingFrequency = FREQUENCY_WARNING;
  } else if (message == "CRITICAL") {
    currentState = STATE_CRITICAL;
    samplingFrequency = FREQUENCY_CRITICAL;
  }
  
  updateLEDs();
}

// ============================================================================
// TELEMETRY DATA COLLECTION
// ============================================================================

void collectTelemetryData() {
  // Read temperature from DS18B20
  sensors.requestTemperatures();
  currentTemperature = sensors.getTempCByIndex(0);
  
  // Simulate OBD-II data
  currentMileage += random(0, 2); // Increment mileage slightly
  
  // Simulate diagnostic codes (10% chance of generating code)
  if (random(100) < 10) {
    String codes[] = {"P0300", "P0420", "P0171", "P0301", "P0441"};
    diagnosticCodes = codes[random(5)];
  } else {
    diagnosticCodes = "";
  }
  
  Serial.println("🌡️  Temperature: " + String(currentTemperature) + "°C");
  Serial.println("🚗 Mileage: " + String(currentMileage) + " km");
  if (diagnosticCodes != "") {
    Serial.println("⚠️  DTC: " + diagnosticCodes);
  }
}

// ============================================================================
// FSM STATE MANAGEMENT
// ============================================================================

void updateSystemState() {
  SystemState previousState = currentState;
  
  if (currentTemperature < TEMP_THRESHOLD_WARNING) {
    currentState = STATE_NORMAL;
    samplingFrequency = FREQUENCY_NORMAL;
  } else if (currentTemperature < TEMP_THRESHOLD_CRITICAL) {
    currentState = STATE_WARNING;
    samplingFrequency = FREQUENCY_WARNING;
  } else {
    currentState = STATE_CRITICAL;
    samplingFrequency = FREQUENCY_CRITICAL;
  }
  
  if (previousState != currentState) {
    Serial.println("🔄 State changed: " + getStateName(previousState) + " → " + getStateName(currentState));
    updateLEDs();
    
    // Notify Control Unit
    mqttClient.publish(mqtt_topic_state, getStateName(currentState).c_str());
  }
}

String getStateName(SystemState state) {
  switch (state) {
    case STATE_NORMAL: return "NORMAL";
    case STATE_WARNING: return "WARNING";
    case STATE_CRITICAL: return "CRITICAL";
    default: return "UNKNOWN";
  }
}

void updateLEDs() {
  digitalWrite(LED_NORMAL, LOW);
  digitalWrite(LED_WARNING, LOW);
  digitalWrite(LED_CRITICAL, LOW);
  
  switch (currentState) {
    case STATE_NORMAL:
      digitalWrite(LED_NORMAL, HIGH);
      break;
    case STATE_WARNING:
      digitalWrite(LED_WARNING, HIGH);
      break;
    case STATE_CRITICAL:
      digitalWrite(LED_CRITICAL, HIGH);
      break;
  }
}

// ============================================================================
// DATA SIGNING (VIN-BASED ECDSA)
// ============================================================================

String buildTelemetryPacket() {
  String packet = "";
  packet += "VIN:" + String(VEHICLE_VIN);
  packet += "|TEMP:" + String(currentTemperature, 2);
  packet += "|MILEAGE:" + String(currentMileage);
  packet += "|STATE:" + getStateName(currentState);
  if (diagnosticCodes != "") {
    packet += "|DTC:" + diagnosticCodes;
  }
  packet += "|TIMESTAMP:" + String(millis());
  
  return packet;
}

String signData(String data) {
  // Simplified signature using SHA256 hash + VIN
  // In production, use proper ECDSA with mbedtls
  
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  
  // Hash data + VIN
  String dataWithVIN = data + String(VEHICLE_VIN);
  mbedtls_md_update(&ctx, (const unsigned char*)dataWithVIN.c_str(), dataWithVIN.length());
  
  unsigned char hash[32];
  mbedtls_md_finish(&ctx, hash);
  mbedtls_md_free(&ctx);
  
  // Convert to hex string
  String signature = "";
  for (int i = 0; i < 32; i++) {
    char hex[3];
    sprintf(hex, "%02x", hash[i]);
    signature += String(hex);
  }
  
  return signature;
}
