// test-mqtt.js
// Quick test to verify MQTT broker connectivity

const mqtt = require('mqtt');
const config = require('./config');

const client = mqtt.connect(`mqtt://${config.MQTT_BROKER}:${config.MQTT_PORT}`);

client.on('connect', () => {
    console.log('✅ Connected to MQTT broker');
    
    // Subscribe to all vehicle topics
    client.subscribe('vehicle/#', (err) => {
        if (!err) {
            console.log('✅ Subscribed to vehicle topics');
        }
    });
    
    // Publish test message
    client.publish('vehicle/test', JSON.stringify({
        message: 'Test from test-mqtt.js',
        timestamp: new Date().toISOString()
    }));
});

client.on('message', (topic, message) => {
    console.log(`📨 [${topic}] ${message.toString()}`);
});

client.on('error', (err) => {
    console.error('❌ MQTT Error:', err.message);
});

// Keep running
console.log('Listening for MQTT messages... (Press Ctrl+C to exit)');
