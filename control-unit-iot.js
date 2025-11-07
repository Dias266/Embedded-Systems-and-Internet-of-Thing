// control-unit-iot.js 
// IoT Integration for Control Unit

const mqtt = require('mqtt');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const express = require('express');
const cors = require('cors');

const app = express();
app.use(cors());
app.use(express.json());

// CONFIGURATION

const MQTT_BROKER = 'mqtt://localhost:1883';
const SERIAL_PORT = '/dev/ttyUSB0'; // CHANGE THIS to your Arduino port!
const SERIAL_BAUD_RATE = 9600;  // Match Arduino baud rate

// MQTT SERVICE

class MQTTService {
    constructor() {
        this.client = mqtt.connect(MQTT_BROKER);
        this.lastTelemetry = null;
        
        this.client.on('connect', () => {
            console.log('MQTT Connected');
            this.client.subscribe('vehicle/telemetry');
            this.client.subscribe('vehicle/state');
        });
        
        this.client.on('message', (topic, message) => {
            this.handleMessage(topic, message.toString());
        });
        
        this.client.on('error', (err) => {
            console.error('MQTT Error:', err.message);
        });
    }
    
    handleMessage(topic, message) {
        console.log(`MQTT [${topic}]: ${message.substring(0, 100)}...`);
        
        if (topic === 'vehicle/telemetry') {
            this.lastTelemetry = message;
            
            // FIXED: Check if serial service is ready before sending
            if (serialService && serialService.port && serialService.port.isOpen) {
                serialService.verifyTelemetry(message);
            } else {
                console.log('Arduino not connected - skipping verification');
            }
            
            // Store telemetry data
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
        console.log(`Published state: ${state}`);
    }
}

// SERIAL SERVICE (Arduino Communication)

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
                console.log('Serial connection established with Arduino');
            });
            
            this.parser.on('data', (data) => {
                this.handleSerialData(data);
            });
            
            this.port.on('error', (err) => {
                console.error('Serial error:', err.message);
                console.log('Arduino not found - system will continue without verification');
            });
            
        } catch (error) {
            console.error('Failed to initialize serial:', error.message);
            console.log('Continuing without Arduino verification');
        }
    }
    
    verifyTelemetry(telemetry) {
        if (this.port && this.port.isOpen) {
            this.port.write(`VERIFY:${telemetry}\n`);
        }
    }
    
    handleSerialData(data) {
        console.log(`Arduino: ${data}`);
        
        if (data.startsWith('AUTH_RESULT:')) {
            const result = data.substring(12).trim();
            
            if (result === 'VALID') {
                console.log('Signature verified by Arduino');
                blockchainService.storeAuthenticatedData(mqttService.lastTelemetry);
            } else {
                console.log('Invalid signature detected');
            }
        }
    }
}

// DATA STORAGE SERVICE

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
        this.updateState(parseFloat(data.temp) || 0);
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
            console.log(`State changed to: ${newState}`);
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
        
        if (temps.length === 0) {
            return { avg: 0, min: 0, max: 0, count: 0 };
        }
        
        return {
            avg: temps.reduce((a, b) => a + b, 0) / temps.length,
            min: Math.min(...temps),
            max: Math.max(...temps),
            count: temps.length
        };
    }
}

// BLOCKCHAIN INTEGRATION SERVICE

class BlockchainService {
    async storeAuthenticatedData(telemetryPacket) {
        try {
            const data = mqttService.parseTelemetry(telemetryPacket);
            
            // Log that data is ready for blockchain
            console.log(' Blockchain storage: Data ready for distributed system');
            console.log('   VIN:', data.vin);
            console.log('   Temp:', data.temp);
            console.log('   Mileage:', data.mileage);
            
            // Optional: Uncomment when distributed systems backend is running
            /*
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
            console.log('Telemetry stored on blockchain:', result);
            */
            
        } catch (error) {
            console.error('Blockchain storage failed:', error.message);
        }
    }
}

// INITIALIZE SERVICES

const mqttService = new MQTTService();
const serialService = new SerialService();
const dataStorageService = new DataStorageService();
const blockchainService = new BlockchainService();

// HTTP API FOR DASHBOARD

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

app.get('/api/status', (req, res) => {
    res.json({
        success: true,
        mqtt: mqttService.client.connected,
        serial: serialService.port ? serialService.port.isOpen : false,
        telemetryCount: dataStorageService.telemetryHistory.length,
        currentState: dataStorageService.currentState,
        timestamp: new Date().toISOString()
    });
});

app.listen(PORT, () => {
    console.log(`\n${'='.repeat(60)}`);
    console.log('IoT Control Unit - Backend Server');
    console.log(`${'='.repeat(60)}\n`);
    console.log(`HTTP Server: http://localhost:${PORT}`);
    console.log(`MQTT Broker: ${MQTT_BROKER}`);
    console.log(`Serial Port: ${SERIAL_PORT}`);
    console.log(`\n Monitoring vehicle telemetry...\n`);
    console.log(` Dashboard: Open dashboard-iot.html in browser`);
    console.log(`\n${'='.repeat(60)}\n`);
});

module.exports = { mqttService, dataStorageService };
