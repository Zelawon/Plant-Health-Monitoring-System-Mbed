#include "ioThread.h"

// Constructor sets up sensors, GPS module, and references to data storage
ioThread::ioThread(LightSensor *lightSensor, MoistureSensor *moistureSensor,
                   GPS *gpsModule, float *soilMoisture, float *brightness,
                   int *mode, int *satelliteCount, float *latitude,
                   float *longitude, char *meridian, char *parallel,
                   float *altitude, char *measurement, char gpsModuleClock[10])
    : lightSensor(lightSensor), moistureSensor(moistureSensor),
      gpsModule(gpsModule), soilMoisture(soilMoisture), brightness(brightness),
      mode(mode), satelliteCount(satelliteCount), latitude(latitude),
      longitude(longitude), meridian(meridian), parallel(parallel),
      altitude(altitude), measurement(measurement),
      gpsModuleClock(gpsModuleClock), _running(true) {}

extern Mutex dataMutex;

// Main function executed by the thread, responsible for gathering data from
// sensors
void ioThread::run() {

  while (true) {
    if (_running) {
      float localBrightness = lightSensor->getBrightness();
      float localSoilMoisture = moistureSensor->getMoisture();

      int localSatelliteCount;
      float localLatitude, localLongitude, localAltitude;
      char localMeridian, localParallel, localMeasurement;
      char localGpsModuleClock[10];

      gpsModule->readData(localSatelliteCount, localLatitude, localLongitude,
                          localMeridian, localParallel, localAltitude,
                          localMeasurement, localGpsModuleClock);

      // Lock mutex before writing shared variables
      dataMutex.lock();
      *brightness = localBrightness;
      *soilMoisture = localSoilMoisture;
      *satelliteCount = localSatelliteCount;
      *latitude = localLatitude;
      *longitude = localLongitude;
      *meridian = localMeridian;
      *parallel = localParallel;
      *altitude = localAltitude;
      *measurement = localMeasurement;
      memcpy(gpsModuleClock, localGpsModuleClock, sizeof(localGpsModuleClock));
      dataMutex.unlock();
      // Apply delay based on mode selection
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
      default:
        ThisThread::sleep_for(NORMAL_INTERVAL); // Fallback delay
      }
    }
  }
}

// Pause the sensor and GPS readings
void ioThread::pause() { _running = false; }

// Resume the sensor and GPS readings
void ioThread::resume() { _running = true; }
