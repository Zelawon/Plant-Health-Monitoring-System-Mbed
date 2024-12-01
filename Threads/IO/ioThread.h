#ifndef IOTHREAD_H
#define IOTHREAD_H

#include "../../Sensors/GPS/GPS.h"
#include "../../Sensors/LightSensor/LightSensor.h"
#include "../../Sensors/MoistureSensor/MoistureSensor.h"
#include "mbed.h"

#define TEST_INTERVAL 2s
#define NORMAL_INTERVAL 10s
#define AVERAGE_INTERVAL 40s

class ioThread {
public:
  // Constructor: Initialize with sensors, GPS, and data pointers
  ioThread(LightSensor *lightSensor, MoistureSensor *moistureSensor,
           GPS *gpsModule, float *soilMoisture, float *brightness, int *mode,
           int *satelliteCount, float *latitude, float *longitude,
           char *meridian, char *parallel, float *altitude, char *measurement,
           char gpsModuleClock[10]);

  void run();    // Thread's main function to collect data
  void pause();  // Pause data collection
  void resume(); // Resume data collection

private:
  LightSensor *lightSensor;
  MoistureSensor *moistureSensor;
  GPS *gpsModule;

  // Pointers to data for storing results
  float *soilMoisture;
  float *brightness;
  int *mode;

  // Pointers to GPS data for storing results
  int *satelliteCount;
  float *latitude;
  float *longitude;
  char *meridian;
  char *parallel;
  float *altitude;
  char *measurement;
  char *gpsModuleClock;

  bool _running; // Flag to control the thread
};

#endif // IOTHREAD_H
