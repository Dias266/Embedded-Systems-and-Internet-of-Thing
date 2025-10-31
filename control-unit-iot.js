// control-unit-iot.js - IoT Integration for Control Unit

const mqtt = require('mqtt');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const express = require('express');
const cors = require('cors');

const app = express();
app.use(cors());
app.use(express.json());

// ============================================================================
// CONFIGURATION
// ============================================================================

const MQTT_BROKER = 'mqtt://localhost:1883';
const SERIAL_PORT = '/dev/ttyUSB0'; // Change to your Arduino port
const SERIAL_BAUD_RATE = 115200;

// ============================================================================
// MQTT SERVICE
// ============================================================================

class MQTTService {
    constructor() {
        this.client = mqtt.connect(MQTT_BROKER);
        this.lastTelemetry = null;
        
        this.client.on('connect', () => {
            console.log('✅ MQTT Connected');
            this.client.subscribe('vehicle/telemetry');
            this.client.subscribe('vehicle/state');
        });
        
        this.client.on('message', (topic, message) => {
            this.handleMessage(topic, message.toString());
        });
    }
    
    handleMessage(topic, message) {
        console.log(`📥 MQTT [${topic}]: ${message.substring(0, 100)}...`);
        
        if (topic === 'vehicle/telemetry') {
            this.lastTelemetry = message;
            // Forward to Arduino for authentication
            serialService.verifyTelemetry(message);
            
            // Store in distributed system
            dataStorageService.storeTelemetry(this.parseTelemetry(message));
        }
    }
    
    parseTelemetry(packet) {
        const parts = packet.split('|');
        const data = {};
        
        parts.forEach(part => {
            if (part.includes(':')) {
                const [key, value] = part.split(':');
                data[key.toLowerCase()] = value;
            }
        });
        
        return data;
    }
    
    publishState(state) {
        this.client.publish('vehicle/state', state);
        console.log(`📤 Published state: ${state}`);
    }
}

// ============================================================================
// SERIAL SERVICE (Arduino Communication)
// ============================================================================

class SerialService {
    constructor() {
        this.port = null;
        this.parser = null;
        this.initializeSerial();
    }
    
    initializeSerial() {
        try {
            this.port = new SerialPort({
                path: SERIAL_PORT,
                baudRate: SERIAL_BAUD_RATE
            });
            
            this.parser = this.port.pipe(new ReadlineParser({ delimiter: '\n' }));
            
            this.port.on('open', () => {
                console.log('✅ Serial connection established with Arduino');
            });
            
            this.parser.on('data', (data) => {
                this.handleSerialData(data);
            });
            
            this.port.on('error', (err) => {
                console.error('❌ Serial error:', err.message);
            });
            
        } catch (error) {
            console.error('❌ Failed to initialize serial:', error.message);
        }
    }
    
    verifyTelemetry(telemetry) {
        if (this.port && this.port.isOpen) {
            this.port.write(`VERIFY:${telemetry}\n`);
        }
    }
    
    handleSerialData(data) {
        console.log(`📥 Arduino: ${data}`);
        
        if (data.startsWith('AUTH_RESULT:')) {
            const result = data.substring(12);
            
            if (result === 'VALID') {
                console.log('✅ Signature verified by Arduino');
                // Forward to blockchain for storage
                blockchainService.storeAuthenticatedData(mqttService.lastTelemetry);
            } else {
                console.log('❌ Invalid signature detected');
            }
        }
    }
}

// ============================================================================
// DATA STORAGE SERVICE
// ============================================================================

class DataStorageService {
    constructor() {
        this.telemetryHistory = [];
        this.maxHistory = 100;
        this.currentState = 'NORMAL';
    }
    
    storeTelemetry(data) {
        this.telemetryHistory.push({
            ...data,
            receivedAt: new Date().toISOString()
        });
        
        // Keep only last N entries
        if (this.telemetryHistory.length > this.maxHistory) {
            this.telemetryHistory = this.telemetryHistory.slice(-this.maxHistory);
        }
        
        // Update state based on temperature
        this.updateState(parseFloat(data.temp));
    }
    
    updateState(temperature) {
        let newState = 'NORMAL';
        
        if (temperature >= 40) {
            newState = 'CRITICAL';
        } else if (temperature >= 30) {
            newState = 'WARNING';
        }
        
        if (newState !== this.currentState) {
            this.currentState = newState;
            console.log(`🔄 State changed to: ${newState}`);
            mqttService.publishState(newState);
        }
    }
    
    getLatestTelemetry() {
        return this.telemetryHistory[this.telemetryHistory.length - 1] || null;
    }
    
    getTelemetryHistory() {
        return this.telemetryHistory;
    }
    
    getStatistics() {
        if (this.telemetryHistory.length === 0) {
            return { avg: 0, min: 0, max: 0, count: 0 };
        }
        
        const temps = this.telemetryHistory
            .map(t => parseFloat(t.temp))
            .filter(t => !isNaN(t));
        
        return {
            avg: temps.reduce((a, b) => a + b, 0) / temps.length,
            min: Math.min(...temps),
            max: Math.max(...temps),
            count: temps.length
        };
    }
}

// ============================================================================
// BLOCKCHAIN INTEGRATION SERVICE
// ============================================================================

class BlockchainService {
    async storeAuthenticatedData(telemetryPacket) {
        try {
            // Parse telemetry
            const data = mqttService.parseTelemetry(telemetryPacket);
            
            // Send to distributed systems backend
            const response = await fetch('http://localhost:3001/api/vehicle/telemetry', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    vin: data.vin,
                    temperature: parseFloat(data.temp),
                    mileage: parseInt(data.mileage),
                    state: data.state,
                    dtc: data.dtc || null,
                    authenticated: true,
                    timestamp: new Date().toISOString()
                })
            });
            
            const result = await response.json();
            console.log('✅ Telemetry stored on blockchain:', result);
            
        } catch (error) {
            console.error('❌ Blockchain storage failed:', error.message);
        }
    }
}

// ============================================================================
// INITIALIZE SERVICES
// ============================================================================

const mqttService = new MQTTService();
const serialService = new SerialService();
const dataStorageService = new DataStorageService();
const blockchainService = new BlockchainService();

// ============================================================================
// HTTP API FOR DASHBOARD
// ============================================================================

const PORT = 3002;

app.get('/api/telemetry/latest', (req, res) => {
    res.json({
        success: true,
        data: dataStorageService.getLatestTelemetry(),
        state: dataStorageService.currentState
    });
});

app.get('/api/telemetry/history', (req, res) => {
    res.json({
        success: true,
        data: dataStorageService.getTelemetryHistory()
    });
});

app.get('/api/telemetry/statistics', (req, res) => {
    res.json({
        success: true,
        stats: dataStorageService.getStatistics(),
        state: dataStorageService.currentState
    });
});

app.listen(PORT, () => {
    console.log(`\n${'='.repeat(60)}`);
    console.log('🚀 IoT Control Unit - Backend Server');
    console.log(`${'='.repeat(60)}\n`);
    console.log(`📍 HTTP Server: http://localhost:${PORT}`);
    console.log(`📡 MQTT Broker: ${MQTT_BROKER}`);
    console.log(`🔌 Serial Port: ${SERIAL_PORT}`);
    console.log(`\n✨ Monitoring vehicle telemetry...\n`);
});

module.exports = { mqttService, dataStorageService };
