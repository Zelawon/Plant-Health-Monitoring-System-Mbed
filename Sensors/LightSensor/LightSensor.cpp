#include "LightSensor.h"

// Constructor: Initializes the AnalogIn pin for the light sensor
LightSensor::LightSensor(PinName pin) : sensor(pin) {}

// Reads brightness level and returns it as a percentage (0% to 100%)
float LightSensor::getBrightness() {
    float analog_value = sensor.read();   // Read analog value (range: 0.0 to 1.0)
    return analog_value * 100.0f;         // Convert to percentage
}
