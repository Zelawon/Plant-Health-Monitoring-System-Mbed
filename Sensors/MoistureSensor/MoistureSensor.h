#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include "mbed.h"

class MoistureSensor {
public:
    // Constructor to initialize the analog pin for the moisture sensor
    MoistureSensor(PinName pin);

    // Returns the moisture level as a percentage (0 to 100)
    float getMoisture();

private:
    AnalogIn sensor;  // Analog input for the moisture sensor
};

#endif // MOISTURE_SENSOR_H
