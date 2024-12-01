#include "ColorSensor.h"
#include "mbed.h"

// Constructor: initialize I2C with specified SDA and SCL pins
ColorSensor::ColorSensor(I2C *i2c, PinName ledPin) : i2c(i2c), ledPin(ledPin) {
  initialize();
}

// Initialize the ColorSensor sensor
void ColorSensor::initialize() {
  writeRegister(TCS34725_ENABLE, TCS34725_ENABLE_PON); // Power ON the sensor
  ThisThread::sleep_for(3ms);                          // Wait for power-on
  writeRegister(TCS34725_ENABLE,
                TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN); // Enable the ADC
  writeRegister(TCS34725_ATIME, 0xC0); // Integration time (50 ms)
  writeRegister(TCS34725_AGAIN, 0x00); // Gain (1x)
}

// Write a value to a specified register
void ColorSensor::writeRegister(uint8_t reg, uint8_t value) {
  char data[2] = {(char)(TCS34725_COMMAND_BIT | reg), (char)value};
  i2c->write(TCS34725_ADDRESS, data, 2);
}

// Read a 16-bit value from a specified register
uint16_t ColorSensor::read16(uint8_t reg) {
  char cmd = TCS34725_COMMAND_BIT | reg;
  char data[2] = {0};

  // Send the register address to read from
  i2c->write(TCS34725_ADDRESS, &cmd, 1);
  i2c->read(TCS34725_ADDRESS, data, 2);

  // Combine the two bytes into a 16-bit value (read word protocol)
  return (data[1] << 8) | data[0];
}

// Read color data (clear, red, green, blue)
void ColorSensor::readColorData(uint16_t &clear, uint16_t &red, uint16_t &green,
                                uint16_t &blue) {
  ledPin = 1;
  ThisThread::sleep_for(3ms);

  clear = read16(TCS34725_CDATAL);
  red = read16(TCS34725_RDATAL);
  green = read16(TCS34725_GDATAL);
  blue = read16(TCS34725_BDATAL);

  ledPin = 0;

  // Brief delay to ensure the sensor has time to refresh its data registers,
  // allowing for stable and accurate color readings in the next measurement
  ThisThread::sleep_for(10ms);
}