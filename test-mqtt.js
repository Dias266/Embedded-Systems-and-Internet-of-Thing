// test-mqtt.js
// Quick test to verify MQTT broker connectivity

const mqtt = require('mqtt');

// FIXED: Use proper function call syntax with parentheses
const client = mqtt.connect('mqtt://localhost:1883');

client.on('connect', () => {
    console.log('Connected to MQTT broker');
    
    // Subscribe to all vehicle topics
    client.subscribe('vehicle/#', (err) => {
        if (!err) {
            console.log('Subscribed to vehicle topics');
        }
    });
    
    // Publish test message
    client.publish('vehicle/test', JSON.stringify({
        message: 'Test from test-mqtt.js',
        timestamp: new Date().toISOString()
    }));
});

client.on('message', (topic, message) => {
    // FIXED: Use proper template literal syntax
    console.log(`[${topic}] ${message.toString()}`);
});

client.on('error', (err) => {
    console.error('MQTT Error:', err.message);
});

// Keep running
console.log('Listening for MQTT messages... (Press Ctrl+C to exit)');
