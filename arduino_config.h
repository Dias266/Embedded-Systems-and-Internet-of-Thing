// arduino_config.h
#ifndef ARDUINO_CONFIG_H
#define ARDUINO_CONFIG_H

// Serial Configuration
#define SERIAL_BAUDRATE 9600

// LCD Configuration
#define LCD_COLS 16
#define LCD_ROWS 2

// LCD Pin Connections
#define LCD_RS 12
#define LCD_EN 11
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

// LED Pin Configuration
#define LED_GREEN 8
#define LED_RED 9
#define LED_YELLOW 10

// Expected VIN for Authentication
#define EXPECTED_VIN "1HGCM82633A123456"

// Timing Configuration
#define DISPLAY_DELAY 3000  // 3 seconds

#endif
