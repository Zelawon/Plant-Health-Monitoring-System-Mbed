#ifndef COLORSENSOR_H
#define COLORSENSOR_H

#include "mbed.h"

// Address
#define TCS34725_ADDRESS            0x29 << 1 // Shifted left for 8-bit format

//  Registers:
#define TCS34725_ENABLE             0x00   // Enable register address
#define TCS34725_ATIME              0x01   // Integration time register address
#define TCS34725_AGAIN              0x0F   // Control register address
#define TCS34725_CDATAL             0x14   // Clear data register address
#define TCS34725_RDATAL             0x16   // Red data register address
#define TCS34725_GDATAL             0x18   // Green data register address
#define TCS34725_BDATAL             0x1A   // Blue data register address

// Commands
#define TCS34725_COMMAND_BIT        0x80   // Command bit for the sensor's registers (read word protocol)
#define TCS34725_ENABLE_PON         0x01   // Power ON command
#define TCS34725_ENABLE_AEN         0x02   // Enable the ADC


class ColorSensor {
public:
    // Constructor
    ColorSensor(I2C *i2c, PinName ledPin);

    // Initialize the sensor
    void initialize();

    // Read color data from the sensor
    void readColorData(uint16_t &clear, uint16_t &red, uint16_t &green, uint16_t &blue);

private:
    // I2C communication object
    I2C *i2c;
    DigitalOut ledPin;
    
    // Helper functions to read and write registers
    void writeRegister(uint8_t reg, uint8_t value);
    uint16_t read16(uint8_t reg);
};

#endif
