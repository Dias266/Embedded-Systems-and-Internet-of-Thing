// test-serial.js
// Test serial communication with Arduino

const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const port = new SerialPort({
    path: '/dev/ttyUSB0',  // Update this!
    baudRate: 9600
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

port.on('open', () => {
    console.log('✅ Serial port opened');
});

parser.on('data', (data) => {
    console.log('📨 Arduino says:', data);
});

port.on('error', (err) => {
    console.error('❌ Serial Error:', err.message);
});

// Send test data
setInterval(() => {
    const testData = JSON.stringify({
        vin: '1HGCM82633A123456',
        temp: 25.5,
        signature: 'test_signature_123'
    });
    
    port.write(testData + '\n', (err) => {
        if (err) {
            console.error('❌ Write error:', err.message);
        } else {
            console.log('📤 Sent to Arduino:', testData);
        }
    });
}, 5000);
