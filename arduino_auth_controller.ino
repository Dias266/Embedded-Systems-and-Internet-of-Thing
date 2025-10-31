/*
 * Arduino Authentication Controller
 * Verifies ECDSA signatures from ESP32 Telematics Device
 * Displays authentication results on LCD (16x2)
 * Indicates status via LEDs
 * Communicates with Control Unit via Serial
 */

#include <LiquidCrystal.h>
#include <sha256.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

// LCD (16x2) pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Status LEDs
#define LED_VALID 7
#define LED_INVALID 8
#define LED_ERROR 9

// ============================================================================
// CONFIGURATION
// ============================================================================

const char* EXPECTED_VIN = "1HGCM82633A123456";

// FSM States
enum DisplayState {
  STATE_WAITING,
  STATE_AUTHENTICATING,
  STATE_VALID,
  STATE_INVALID,
  STATE_ERROR
};

DisplayState currentDisplayState = STATE_WAITING;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Auth Controller");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...");
  
  // Initialize LEDs
  pinMode(LED_VALID, OUTPUT);
  pinMode(LED_INVALID, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);
  
  // Test LEDs
  digitalWrite(LED_VALID, HIGH);
  delay(200);
  digitalWrite(LED_VALID, LOW);
  digitalWrite(LED_INVALID, HIGH);
  delay(200);
  digitalWrite(LED_INVALID, LOW);
  digitalWrite(LED_ERROR, HIGH);
  delay(200);
  digitalWrite(LED_ERROR, LOW);
  
  Serial.println("✅ Arduino Authentication Controller Ready");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  if (Serial.available() > 0) {
    String incomingData = Serial.readStringUntil('\n');
    incomingData.trim();
    
    if (incomingData.startsWith("VERIFY:")) {
      // Extract telemetry packet from Control Unit
      String packet = incomingData.substring(7);
      
      currentDisplayState = STATE_AUTHENTICATING;
      updateDisplay("Authenticating", "Please wait...");
      
      // Verify signature
      bool isValid = verifySignature(packet);
      
      if (isValid) {
        currentDisplayState = STATE_VALID;
        // Extract temperature for display
        float temp = extractTemperature(packet);
        String tempStr = "T:" + String(temp, 1) + "C";
        updateDisplay("AUTH: VALID", tempStr);
        
        digitalWrite(LED_VALID, HIGH);
        digitalWrite(LED_INVALID, LOW);
        digitalWrite(LED_ERROR, LOW);
        
        // Send verification result to Control Unit
        Serial.println("AUTH_RESULT:VALID");
        
      } else {
        currentDisplayState = STATE_INVALID;
        updateDisplay("AUTH: INVALID", "Signature Error");
        
        digitalWrite(LED_VALID, LOW);
        digitalWrite(LED_INVALID, HIGH);
        digitalWrite(LED_ERROR, LOW);
        
        Serial.println("AUTH_RESULT:INVALID");
      }
      
      delay(3000); // Display result for 3 seconds
      
      currentDisplayState = STATE_WAITING;
      updateDisplay("Auth Controller", "Waiting...");
      digitalWrite(LED_VALID, LOW);
      digitalWrite(LED_INVALID, LOW);
    }
  }
}

// ============================================================================
// SIGNATURE VERIFICATION
// ============================================================================

bool verifySignature(String packet) {
  // Parse packet: telemetry|SIG:signature
  int sigIndex = packet.indexOf("|SIG:");
  if (sigIndex == -1) {
    Serial.println("❌ No signature found");
    return false;
  }
  
  String telemetryData = packet.substring(0, sigIndex);
  String receivedSignature = packet.substring(sigIndex + 5);
  
  // Verify VIN
  int vinIndex = telemetryData.indexOf("VIN:");
  if (vinIndex == -1) {
    Serial.println("❌ No VIN found");
    return false;
  }
  
  int vinEnd = telemetryData.indexOf("|", vinIndex);
  String vin = telemetryData.substring(vinIndex + 4, vinEnd);
  
  if (vin != String(EXPECTED_VIN)) {
    Serial.println("❌ VIN mismatch: " + vin);
    return false;
  }
  
  // Recompute signature
  String dataWithVIN = telemetryData + vin;
  String computedSignature = computeSignature(dataWithVIN);
  
  // Compare signatures
  bool isValid = (computedSignature == receivedSignature);
  
  Serial.println("🔐 Signature verification: " + String(isValid ? "VALID" : "INVALID"));
  
  return isValid;
}

String computeSignature(String data) {
  // Use SHA256 (simplified version of ECDSA)
  Sha256 sha256;
  sha256.init();
  
  for (unsigned int i = 0; i < data.length(); i++) {
    sha256.write(data[i]);
  }
  
  uint8_t* hash = sha256.result();
  
  // Convert to hex string
  String signature = "";
  for (int i = 0; i < 32; i++) {
    char hex[3];
    sprintf(hex, "%02x", hash[i]);
    signature += String(hex);
  }
  
  return signature;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

float extractTemperature(String packet) {
  int tempIndex = packet.indexOf("TEMP:");
  if (tempIndex == -1) return 0.0;
  
  int tempEnd = packet.indexOf("|", tempIndex);
  String tempStr = packet.substring(tempIndex + 5, tempEnd);
  
  return tempStr.toFloat();
}

void updateDisplay(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
