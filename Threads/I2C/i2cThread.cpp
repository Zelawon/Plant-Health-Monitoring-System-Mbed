#include "i2cThread.h"
#include "mbed.h"
#include <cstdint>

// Constructor initializes thread with pointers to sensor instances and data
// storage
i2cThread::i2cThread(I2C *i2c, Accelerometer *accelerometer,
                     Si7021 *humidityTemperature, ColorSensor *rgbColorSensor,
                     float *xAxis, float *yAxis, float *zAxis,
                     float *temperature, float *humidity, int *clear, int *red,
                     int *green, int *blue, int *mode)
    : i2c(i2c), accelerometer(accelerometer),
      humidityTemperature(humidityTemperature), rgbColorSensor(rgbColorSensor),
      xAxis(xAxis), yAxis(yAxis), zAxis(zAxis), temperature(temperature),
      humidity(humidity), clear(clear), red(red), green(green), blue(blue),
      mode(mode), _running(true) {}

// Primary function run by thread: gathers sensor data and assigns it to
// variables

extern Mutex dataMutex;

void i2cThread::run() {

  while (true) {
    if (_running) {
      // Get accelerometer readings and store in respective variables
      float localXAxis = accelerometer->getX();
      float localYAxis = accelerometer->getY();
      float localZAxis = accelerometer->getZ();

      float localTemperature = humidityTemperature->readTemperature();
      float localHumidity = humidityTemperature->readHumidity();

      uint16_t rawClear, rawRed, rawGreen, rawBlue;
      rgbColorSensor->readColorData(rawClear, rawRed, rawGreen, rawBlue);

      int localClear = static_cast<int>(rawClear);
      int localRed = static_cast<int>(rawRed);
      int localGreen = static_cast<int>(rawGreen);
      int localBlue = static_cast<int>(rawBlue);

      // Lock mutex before writing shared variables
      dataMutex.lock();
      *xAxis = localXAxis;
      *yAxis = localYAxis;
      *zAxis = localZAxis;
      *temperature = localTemperature;
      *humidity = localHumidity;
      *clear = localClear;
      *red = localRed;
      *green = localGreen;
      *blue = localBlue;
      dataMutex.unlock();

      // Adjust delay based on mode: TEST, NORMAL, or ADVANCED
      switch (*mode) {
      case 0:
        ThisThread::sleep_for(TEST_INTERVAL);
        break;
      case 1:
        ThisThread::sleep_for(NORMAL_INTERVAL);
        break;
      case 2:
        ThisThread::sleep_for(NORMAL_INTERVAL);
        break;
      }
    }
  }
}

// Pause the sensor readings
void i2cThread::pause() { _running = false; }

// Resume the sensor readings
void i2cThread::resume() { _running = true; }
