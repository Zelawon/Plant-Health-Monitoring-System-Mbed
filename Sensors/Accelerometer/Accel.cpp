#include "Accel.h"
#include "mbed.h"

// Constructor: initialize I2C communication for accelerometer
Accelerometer::Accelerometer(I2C *i2c) : i2c(i2c) {
    initialize(); 
}

// Initialize accelerometer with basic settings
void Accelerometer::initialize() {
    // Set accelerometer to active mode, 14-bit data output
    uint8_t data[2] = {REG_CTRL_1, 0x01}; 
    writeRegister(data, 2);  // Write control data to control register
}

// Retrieve WHO_AM_I register value to confirm communication
uint8_t Accelerometer::getId() {
    uint8_t whoAmI = 0;
    readRegister(MMA8451_WHO_AM_I, &whoAmI);
    return whoAmI;
}

// Retrieve X-axis acceleration in m/s²
float Accelerometer::getX() {
    return readAxis(MMA8451_OUT_X_MSB);  // Read X-axis register
}

// Retrieve Y-axis acceleration in m/s²
float Accelerometer::getY() {
    return readAxis(MMA8451_OUT_Y_MSB);  // Read Y-axis register
}

// Retrieve Z-axis acceleration in m/s²
float Accelerometer::getZ() {
    return readAxis(MMA8451_OUT_Z_MSB);  // Read Z-axis register
}

// Read a single register from the accelerometer
bool Accelerometer::readRegister(uint8_t reg, uint8_t *data) {
    // Write register address and read single byte into `data`
    int status = i2c->write(MMA8451_ADDRESS, (char*)&reg, 1, true);
    status |= i2c->read(MMA8451_ADDRESS, (char*)data, 1, false);
    return (status == 0);  // Return success status
}

// Write data to a specified register
void Accelerometer::writeRegister(uint8_t *data, int len) {
    i2c->write(MMA8451_ADDRESS, (char*)data, len);  // Write bytes to I2C
}

// Read and calculate acceleration for a specific axis
float Accelerometer::readAxis(uint8_t reg) {
    char buf[2] = {0}; // Buffer for MSB and LSB

    // Request 2 bytes from the specified axis register
    // The write operation sends the register address, enabling the read operation to fetch data from it.
    i2c->write(MMA8451_ADDRESS, (char*)&reg, 1, true);
    i2c->read(MMA8451_ADDRESS, buf, 2, false);

    // Combine MSB and LSB, shifting to get 14-bit signed data
    int16_t axisData = (static_cast<int16_t>(buf[0]) << 8 | buf[1]) >> 2;

    // Adjust for 14-bit signed data (compensate if overflowed)
    // If the data exceeds the positive range of 14-bit values, adjust by wrapping it to the correct signed range.
    if (axisData > UINT14_MAX / 2) {
        axisData -= UINT14_MAX + 1; // Convert overflowed value to its negative counterpart
    }

    // Convert to acceleration in m/s² (1g ≈ 9.81 m/s²)
    constexpr float scale_factor = 9.81f / 4096.0f;
    return static_cast<float>(axisData) * scale_factor;
}
