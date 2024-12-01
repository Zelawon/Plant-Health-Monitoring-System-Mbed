#ifndef ACCEL_H
#define ACCEL_H

#include "mbed.h"

// MMA8451 Register Definitions
#define MMA8451_WHO_AM_I           0x0D     // Device ID Register
#define MMA8451_ADDRESS            0x1D << 1 // I2C address (shifted for R/W bit), SA0 = 1
#define REG_CTRL_1                 0x2A     // Control Register 1
#define MMA8451_OUT_X_MSB          0x01     // X-axis data (Most Significant Byte)
#define MMA8451_OUT_X_LSB          0x02     // X-axis data (Least Significant Byte)
#define MMA8451_OUT_Y_MSB          0x03     // Y-axis data (Most Significant Byte)
#define MMA8451_OUT_Y_LSB          0x04     // Y-axis data (Least Significant Byte)
#define MMA8451_OUT_Z_MSB          0x05     // Z-axis data (Most Significant Byte)
#define MMA8451_OUT_Z_LSB          0x06     // Z-axis data (Least Significant Byte)
#define UINT14_MAX                 16383    // Max 14-bit value (2^14 - 1)

class Accelerometer {
public:
    // Constructor, Initializes I2C communication
    Accelerometer(I2C *i2c);

    // Initialize the accelerometer
    void initialize();

    // Get WHO_AM_I register value to verify device ID
    uint8_t getId();

    // Get accelerometer readings in m/s²
    float getX();
    float getY();
    float getZ();

private:
    // I2C object for communication
    I2C *i2c;

    // Read a single register
    bool readRegister(uint8_t reg, uint8_t *data);

    // Write to a register
    void writeRegister(uint8_t *data, int len);

    // Read axis data
    float readAxis(uint8_t reg);
};

#endif 
