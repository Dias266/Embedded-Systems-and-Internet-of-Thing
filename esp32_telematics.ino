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
#include "config.h"

// CONFIGURATION - Using config.h

// WiFi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// MQTT Configuration FIXED Use IP Adress not "localhost"
const char* mqtt_server = MQTT_BROKER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_topic_telemetry = MQTT_TOPIC_TELEMETRY;
const char* mqtt_topic_state = MQTT_TOPIC_STATE;

// VIN Configuration (for signing)
const char* VEHICLE_VIN = TEST_VIN;

// Temperature sensor
#define ONE_WIRE_BUS 4
#define LED_NORMAL 2
#define LED_WARNING 5
#define LED_CRITICAL 18

// Temperature thresholds
#define TEMP_THRESHOLD_WARNING TEMP_NORMAL
#define TEMP_THRESHOLD_CRITICAL TEMP_WARNING

// Sampling frequencies 
#define FREQUENCY_NORMAL FREQ_NORMAL
#define FREQUENCY_WARNING FREQ_WARNING
#define FREQUENCY_CRITICAL FREQ_CRITICAL

// GLOBAL VARIABLES

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

// SETUP
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
  
  Serial.println("ESP32 Telematics Device Initialized");
  Serial.println("VIN: " + String(VEHICLE_VIN));
  
  updateLEDs();
}

// MAIN LOOP

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
    
    Serial.println("Telemetry sent: " + telemetryData);
    Serial.println("Signature: " + signature.substring(0, 16) + "...");
  }
}

// WIFI CONNECTION

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
    Serial.println("\nWiFi connected");
    Serial.println("IP: " + WiFi.localIP().toString());
    digitalWrite(LED_NORMAL, HIGH);
    delay(1000);
    digitalWrite(LED_NORMAL, LOW);
  } else {
    Serial.println("\nWiFi connection failed");
    Serial.println("Check your SSID and password in config.h");
    digitalWrite(LED_CRITICAL, HIGH);
  }
}

// MQTT CONNECTION

void connectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT");
    
    String clientId = "ESP32_" + String(VEHICLE_VIN);
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT connected");
      mqttClient.subscribe(mqtt_topic_state);
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.println(mqttClient.state());
      Serial.println("Check MQTT_BROKER IP in config.h");
      delay(2000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("MQTT received: " + message);
  
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

// TELEMETRY DATA COLLECTION

void collectTelemetryData() {
  // Read temperature from DS18B20
  sensors.requestTemperatures();
  currentTemperature = sensors.getTempCByIndex(0);

  // Check for sensor errors
  if (currentTemperature == -127.0 || currentTemperature == 85.0) {
    Serial.println("Temperature sensor error!");
    currentTemperature = 25.0; // Use default value
  }
  
  // Simulate OBD-II data
  currentMileage += random(0, 2); // Increment mileage slightly
  
  // Simulate diagnostic codes (10% chance of generating code)
  if (random(100) < 10) {
    String codes[] = {"P0300", "P0420", "P0171", "P0301", "P0441"};
    diagnosticCodes = codes[random(5)];
  } else {
    diagnosticCodes = "";
  }
  
  Serial.println("Temperature: " + String(currentTemperature) + "°C");
  Serial.println("Mileage: " + String(currentMileage) + " km");
  if (diagnosticCodes != "") {
    Serial.println("DTC: " + diagnosticCodes);
  }
}

// FSM STATE MANAGEMENT

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
    Serial.println("State changed: " + getStateName(previousState) + " → " + getStateName(currentState));
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

// DATA SIGNING (VIN-BASED)

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
  // DJB2 hash algorithm - matches Arduino implementation
  unsigned long hash = 5381;
  String dataWithVIN = data + String(VEHICLE_VIN);
  
  for (unsigned int i = 0; i < dataWithVIN.length(); i++) {
    hash = ((hash << 5) + hash) + dataWithVIN[i]; // hash * 33 + c
  }
  
  // Convert to hex string (8 characters)
  char hashStr[9];
  sprintf(hashStr, "%08lx", hash);
  return String(hashStr);
}
  
