#ifndef I2C_THREAD_H
#define I2C_THREAD_H

#include "../../Sensors/Accelerometer/Accel.h"
#include "../../Sensors/Color-Sensor/ColorSensor.h"
#include "../../Sensors/Humidity-Temperature/Si7021.h"
#include "mbed.h"

#define TEST_INTERVAL 2s
#define NORMAL_INTERVAL 10s
#define AVERAGE_INTERVAL 40s

class i2cThread {
public:
  // Constructor: Takes a pointer to the sensors
  i2cThread(I2C *i2c, Accelerometer *accelerometer, Si7021 *humidityTemperature,
            ColorSensor *rgbColorSensor, float *xAxis, float *yAxis,
            float *zAxis, float *temperature, float *humidity, int *clear,
            int *red, int *green, int *blue, int *mode);

  // Thread's run method to handle sensor measurements
  void run();

  // Pause the measurements
  void pause();

  // Resume the measurements
  void resume();

private:
  I2C *i2c;

  // Sensors
  Accelerometer *accelerometer;
  Si7021 *humidityTemperature;
  ColorSensor *rgbColorSensor;

  // Pointers to data variables
  float *xAxis, *yAxis, *zAxis;
  float *temperature, *humidity;
  int *clear, *red, *green, *blue;

  // Mode
  int *mode;

  // Used to control pause-resume taking measurements
  bool _running;
};

#endif // I2C_THREAD_H
