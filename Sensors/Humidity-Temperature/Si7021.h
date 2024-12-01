#ifndef SI7021_H
#define SI7021_H

#include "mbed.h"

// Si7021 I2C address and commands
#define SI7021_ADDR           (0x40 << 1)   // Device I2C address (shifted for 8-bit format)
#define CMD_MEASURE_TEMP      0xE3          // Command to measure temperature in hold mode
#define CMD_MEASURE_HUMIDITY  0xE5          // Command to measure humidity in hold mode

class Si7021 {
public:
    // Constructor: takes a pointer to an I2C interface for communication
    Si7021(I2C *i2c);

    // Read temperature in Celsius; returns -1.0f if an error occurs
    float readTemperature();

    // Read relative humidity as a percentage; returns -1.0f if an error occurs
    float readHumidity();

private:
    I2C *i2c;  // Pointer to the I2C interface used for communication

    //read  16-bit data from the sensor for the specified command
    uint16_t readData(char command);
};

#endif // SI7021_H
