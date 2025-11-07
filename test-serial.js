// test-serial.js 
// Test serial communication with Arduino

const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const port = new SerialPort({
    path: '/dev/ttyUSB0',  // CHANGE THIS to your Arduino port!
    baudRate: 9600  // FIXED: Match Arduino baud rate
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

port.on('open', () => {
    console.log('Serial port opened');
});

parser.on('data', (data) => {
    console.log('📨 Arduino says:', data);
});

port.on('error', (err) => {
    console.error('Serial Error:', err.message);
    console.log('\n Tips:');
    console.log('   1. Check Arduino is plugged in');
    console.log('   2. Update SERIAL_PORT in this file');
    console.log('   3. Close Arduino IDE Serial Monitor');
    console.log('   4. On Linux: sudo usermod -a -G dialout $USER');
});

// Send test data every 5 seconds
setInterval(() => {
    const testData = JSON.stringify({
        vin: '1HGCM82633A123456',
        temp: 25.5,
        signature: 'test_signature_123'
    });
    
    port.write(testData + '\n', (err) => {
        if (err) {
            console.error(' Write error:', err.message);
        } else {
            console.log(' Sent to Arduino:', testData);
        }
    });
}, 5000);
