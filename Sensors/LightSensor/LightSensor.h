#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include "mbed.h"

class LightSensor {
public:
    // Constructor: Initializes the analog input pin for the light sensor
    LightSensor(PinName pin);

    // Reads the brightness level and returns it as a percentage (0 to 100)
    float getBrightness();

private:
    AnalogIn sensor;  // AnalogIn object representing the light sensor input
};

#endif // LIGHT_SENSOR_H
