#include "mbed.h"
#include "Si7021.h"

// Constructor
Si7021::Si7021(I2C *i2c) : i2c(i2c) {}

// Read a 16-bit value from the sensor for a given command
uint16_t Si7021::readData(char command) {
    char data[2] = {0};  // Buffer

    // Send the command to the sensor to initiate measurment 
    if (i2c->write(SI7021_ADDR, &command, 1) != 0) {
        return 0;  // Return 0 if write fails
    }

    // Read 2 bytes of measurement data
    if (i2c->read(SI7021_ADDR, data, 2) != 0) {
        return 0;  // Return 0 if read fails
    }

    // Combine the two bytes to form a single 16-bit result
    return (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);
}

// Read temperature in Celsius, with error handling
float Si7021::readTemperature() {
    uint16_t raw_temperature = readData(CMD_MEASURE_TEMP);

    // If reading failed, return an error code
    if (raw_temperature == 0) {
        return -1.0f;  // Error indicator
    }

    // Convert raw temperature data to Celsius using formula from datasheet
    constexpr float SCALE_TEMP = 175.72f / 65536.0f;  // Precomputed scale factor
    return (raw_temperature * SCALE_TEMP) - 46.85f;
}

// Read relative humidity as a percentage, with error handling
float Si7021::readHumidity() {
    uint16_t raw_humidity = readData(CMD_MEASURE_HUMIDITY);

    // If reading failed, return an error code
    if (raw_humidity == 0) {
        return -1.0f;  // Error indicator
    }

    // Convert raw humidity data to percentage using formula from datasheet
    constexpr float SCALE_HUMID = 125.0f / 65536.0f;  // Precomputed scale factor
    return (raw_humidity * SCALE_HUMID) - 6.0f;
}
