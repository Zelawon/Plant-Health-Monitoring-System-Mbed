#include "MoistureSensor.h"

// Constructor: Initializes the AnalogIn pin for the moisture sensor
MoistureSensor::MoistureSensor(PinName pin) : sensor(pin) {}

// Reads moisture level and returns it as a percentage (0% to 100%)
float MoistureSensor::getMoisture() {
    float analog_value = sensor.read();    // Read analog value (range: 0.0 to 1.0)
    return analog_value * 100.0f;          // Convert to percentage
}
