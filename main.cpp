#include "mbed.h"
#include <cstdio>
#include <string>
#include <vector>

// Include sensor headers
#include "/Sensors/GPS/GPS.h"
#include "/Sensors/MoistureSensor/MoistureSensor.h"
#include "Sensors/Accelerometer/Accel.h"
#include "Sensors/Color-Sensor/ColorSensor.h"
#include "Sensors/Humidity-Temperature/Si7021.h"
#include "Sensors/LightSensor/LightSensor.h"

// Include thread headers
#include "Threads/I2C/i2cThread.h"
#include "Threads/IO/ioThread.h"
#include "Threads/PWM/pwmThread.h"

// Include structs
#include "Structs/Violation.h"

// Define delays for different operation modes
#define TEST_INTERVAL 2s
#define NORMAL_INTERVAL 10s
#define AVERAGE_INTERVAL 40s

// Define thresholds
#define TEMP_MIN -10.0
#define TEMP_MAX 30.0
#define HUMIDITY_MIN 25.0
#define HUMIDITY_MAX 90.0
#define LIGHT_MIN 0.0
#define LIGHT_MAX 80.0
#define MOISTURE_MIN 0.0
#define MOISTURE_MAX 80.0
#define ACCEL_X_LIMIT 2.0
#define ACCEL_Y_LIMIT 2.0
#define ACCEL_Z_LIMIT 11.0

// Setup I2C and Serial communication
static I2C i2c(PB_9, PB_8);      // (SDA, SCL) for I2C communication
BufferedSerial pc(USBTX, USBRX); // Serial communication for debugging

// Instantiate sensor objects with appropriate pins
// IO Sensors
GPS gps(PA_9, PA_10);                // GPS on PA_9 (TX) and PA_10 (RX)
LightSensor lightSensor(PA_4);       // Light sensor on PA_4
MoistureSensor moistureSensor(PA_0); // Moisture sensor on PA_0

// I2C sensors
Accelerometer accelerometer(&i2c); // Accelerometer using I2C
Si7021 humidityTemperature(&i2c);  // Temperature and humidity sensor using I2C
ColorSensor rgbColorSensor(
    &i2c,
    PB_13); // RGB color sensor using I2C, PB_13 for turning LED on and off

// RGB LED for visual feedback
DigitalOut redLED(PB_15), greenLED(PA_12), blueLED(PA_11);

// Board LEDs ffor mode indication
DigitalOut led1(PB_5), led2(PA_5), led3(PB_6);

// USER button to switch between modes
InterruptIn userButton(PB_2);

// Enumeration for operating modes
enum Mode { TEST, NORMAL, ADVANCED };
Mode currentMode = TEST, newMode; // Current and new mode variables
int threadMode = 0;               // Controls thread delay based on mode

// Flags and variables for managing periodic actions
volatile bool changeModeRequest =
    false; // Flag indicating a mode change is requested
volatile bool printMeasurementsFlag =
    false; // Flag to trigger periodic printing
volatile bool printAveragesFlag = false;
Ticker measurementTicker; // Ticker for periodic actions
Ticker averageTicker;     // NORMAL MODE: For printing averages

// Sensor data variables for storing sensor readings
float xAxis = 0, yAxis = 0, zAxis = 0;       // Accelerometer data
float temperature = 0, humidity = 0;         // Temperature and humidity data
int clear = 0, red = 0, green = 0, blue = 0; // RGB color sensor data
float moisture = 0, brightness = 0; // Soil moisture and light brightness data
int satelliteCount = 0;             // GPS satellite count
float latitude = 0.0, longitude = 0.0, altitude = 0.0;  // GPS location data
char meridian = ' ', parallel = ' ', measurement = ' '; // GPS directional data
char gpsModuleClock[10] = {0};                          // GPS time data

// Statistical variables for NORMAL mode
float tempSum = 0, tempMin = 0, tempMax = 0;
float humSum = 0, humMin = 0, humMax = 0;
float xAxisSum = 0, xAxisMin = 0, xAxisMax = 0;
float yAxisSum = 0, yAxisMin = 0, yAxisMax = 0;
float zAxisSum = 0, zAxisMin = 0, zAxisMax = 0;
float lightSum = 0, lightMin = 0, lightMax = 0;
float soilSum = 0, soilMin = 0, soilMax = 0;
int sampleCount = 0;

// For dominant color counts
int redCount = 0, greenCount = 0, blueCount = 0;

bool tempViolated = false;
bool humidityViolated = false;
bool lightViolated = false;
bool moistureViolated = false;
bool accelViolated = false;

bool violationsActive = false;

Violation activeViolations[MAX_VIOLATIONS]; // List of current active violations
size_t violationCount = 0;
size_t currentViolationIndex = 0;

Mutex dataMutex;

// Button and Mode Handling
void buttonPressed();             // Button press handler
void changeMode();                // Function to handle mode switching
void modeOperation();             // Perform mode-specific operations
void measurementTickerCallback(); // Callback to trigger periodic data printing
void averageTickerCallback(); // Callback for printing averages in NORMAL mode

// LED and Sensor State Management
void updateLEDs(); // Update LEDs based on sensor readings or mode
void setLEDState(bool redState, bool greenState,
                 bool blueState); // Control LED states
bool checkLimit(const char *sensorName, float value, float minLimit,
                float maxLimit, bool &violationFlag, bool redState,
                bool greenState, bool blueState); // Check sensor limits
void handleDominantColor(); // Handle dominant color in TEST mode
void handleNoViolations();  // Handle normal state with no violations

// Dominant Color Handling
void updateDominantColorCount(
    int localRed, int localGreen,
    int localBlue); // Update dominant color counts in TEST mode

// Printing Functions
void printMode();                 // Print the current mode to the console
void printMeasurements();         // Print real-time sensor measurements
void printAverages();             // Print sensor averages in NORMAL mode
void printDominantColorSummary(); // Print dominant color summary

void updateStats(float temp, float hum, float x, float y, float z, float light,
                 float soil);
void resetStats();

i2cThread i2cThreadObj(&i2c, &accelerometer, &humidityTemperature,
                       &rgbColorSensor, &xAxis, &yAxis, &zAxis, &temperature,
                       &humidity, &clear, &red, &green, &blue, &threadMode);

ioThread ioThreadObj(&lightSensor, &moistureSensor, &gps, &moisture,
                     &brightness, &threadMode, &satelliteCount, &latitude,
                     &longitude, &meridian, &parallel, &altitude, &measurement,
                     gpsModuleClock);

pwmThread pwmThreadObj(&redLED, &greenLED, &blueLED, &red, &green, &blue,
                       &clear, &threadMode, &violationsActive, activeViolations,
                       &violationCount, &rgbColorSensor, &dataMutex);

int main() {
  // ALL MODES: Set baud rate for serial communication
  pc.set_baud(9600);

  setLEDState(true, true, true);

  // ALL MODES: Initialize I2C and IO sensor threads
  Thread thread_I2C(osPriorityNormal, 1024);
  thread_I2C.start(callback(&i2cThreadObj, &i2cThread::run));

  Thread thread_Analog(osPriorityNormal, 1024);
  thread_Analog.start(callback(&ioThreadObj, &ioThread::run));

  Thread thread_PWM(osPriorityLow, 1024);
  thread_PWM.start(callback(&pwmThreadObj, &pwmThread::run));

  // ALL MODES: Set up USER button to trigger mode change when pressed
  userButton.mode(PullUp);        // Configure button with pull-up resistor
  userButton.fall(buttonPressed); // Trigger on falling edge (button press)

  // Set up periodic action (to print sensor data) using ticker
  measurementTicker.attach(&measurementTickerCallback, TEST_INTERVAL);

  printMode();

  while (true) {

    if (changeModeRequest) {
      changeMode();
      printMode();
    }

    modeOperation();

    ThisThread::sleep_for(100ms);
  }
}

// ALL MODES: Function triggered when the USER button is pressed
void buttonPressed() {
  changeModeRequest = true; // Set flag to indicate mode change is requested
}

// ALL MODES: Callback function to periodically set the print flag
void measurementTickerCallback() { printMeasurementsFlag = true; }

void averageTickerCallback() {
  if (currentMode == NORMAL || currentMode == NORMAL) {
    printAveragesFlag = true;
  }
}

// ALL MODES: Function to handle operations that should be done periodically
// based on the mode
void modeOperation() {

  // Handle periodic operations based on mode
  if (currentMode == TEST) {
    if (printMeasurementsFlag) {
      updateLEDs();
      printf("\n>>> TEST MODE: Updating LEDs and Printing Measurements <<<\n");
      printMeasurements(); // Print live sensor data
    }
  } else if (currentMode == NORMAL || currentMode == ADVANCED) {
    if (printMeasurementsFlag) {
      dataMutex.lock();
      // Read sensor data
      float localTemperature = temperature;
      float localHumidity = humidity;
      float localXAxis = xAxis;
      float localYAxis = yAxis;
      float localZAxis = zAxis;
      float localBrightness = brightness;
      float localSoilMoisture = moisture;
      int localRed = red;
      int localGreen = green;
      int localBlue = blue;
      dataMutex.unlock();

      updateLEDs();
      updateStats(localTemperature, localHumidity, localXAxis, localYAxis,
                  localZAxis, localBrightness, localSoilMoisture);
      updateDominantColorCount(localRed, localGreen, localBlue);
      printMeasurements();
    }
    if (printAveragesFlag) {
      printf("\n>>> NORMAL MODE: Printing Averages <<<\n");
      printAverages();
      resetStats();
    }
  } else {
    printf("\n>>> ERROR: No mode active, RESTART the board <<<\n");
  }

  printMeasurementsFlag = false;
  printAveragesFlag = false;
}

// NORMAL MODE: Keeps track of what color was recorded the most
void updateDominantColorCount(int r, int g, int b) {
  if (r > g && r > b) {
    redCount++;
  } else if (g > r && g > b) {
    greenCount++;
  } else if (b > r && b > g) {
    blueCount++;
  }
}

void updateLEDs() {
    static Timer iterationTimer;

    // TEST Mode: Handle dominant color detection
    if (currentMode == TEST) {
        handleDominantColor();
        return; // Skip other checks in TEST mode
    }

    // Start or reset the timer for NORMAL/ADVANCED modes
    if (!iterationTimer.elapsed_time().count()) {
        iterationTimer.start();
    }

    dataMutex.lock();
    // Reset the violation tracking variables
    violationCount = 0;

    // Check sensor limits and populate active violations
    checkLimit("Temperature", temperature, TEMP_MIN, TEMP_MAX, tempViolated, true, false, false);
    checkLimit("Humidity", humidity, HUMIDITY_MIN, HUMIDITY_MAX, humidityViolated, false, true, false);
    checkLimit("Light Intensity", brightness, LIGHT_MIN, LIGHT_MAX, lightViolated, false, false, true);
    checkLimit("Soil Moisture", moisture, MOISTURE_MIN, MOISTURE_MAX, moistureViolated, false, true, true);
    checkLimit("Acceleration (X)", xAxis, -ACCEL_X_LIMIT, ACCEL_X_LIMIT, accelViolated, true, false, false);
    checkLimit("Acceleration (Y)", yAxis, -ACCEL_Y_LIMIT, ACCEL_Y_LIMIT, accelViolated, true, true, false);
    checkLimit("Acceleration (Z)", zAxis, -ACCEL_Z_LIMIT, ACCEL_Z_LIMIT, accelViolated, true, true, false);

    // Update the global violation state
    violationsActive = (violationCount > 0);
    dataMutex.unlock();

    printf("updateLEDs: Violations Active = %d, Violation Count = %zu\n",
           violationsActive, violationCount);
}


bool checkLimit(const char *sensorName, float value, float minLimit, float maxLimit,
                bool &violationFlag, bool redState, bool greenState, bool blueState) {
    bool currentViolation = (value < minLimit || value > maxLimit);

    if (currentViolation) {
        if (!violationFlag) {
            // First-time violation detected
            printf("%s out of range: %.2f (Limits: %.2f to %.2f)\n", sensorName, value, minLimit, maxLimit);
            violationFlag = true;
        }

        // Add the violation to the array if space is available
        if (violationCount < MAX_VIOLATIONS) {
            activeViolations[violationCount++] = {sensorName, redState, greenState, blueState};
        }
    } else if (violationFlag) {
        // Violation resolved
        printf("%s back within range: %.2f\n", sensorName, value);
        violationFlag = false;
    }

    return currentViolation;
}

void setLEDState(bool redState, bool greenState, bool blueState) {
  redLED = redState ? 1 : 0;
  greenLED = greenState ? 1 : 0;
  blueLED = blueState ? 1 : 0;
}

// Function to handle dominant color detection in TEST mode
void handleDominantColor() {
  // Ensure only one LED is active based on the dominant color
  if (red > green && red > blue) {
    redLED = 0;
    greenLED = 1;
    blueLED = 1; // Red is dominant
  } else if (green > red && green > blue) {
    redLED = 1;
    greenLED = 0;
    blueLED = 1; // Green is dominant
  } else if (blue > red && blue > green) {
    redLED = 1;
    greenLED = 1;
    blueLED = 0; // Blue is dominant
  } else {
    redLED = greenLED = blueLED = 0; // No dominant color
  }
}

// ALL MODES
void changeMode() {
  measurementTicker.detach();
  averageTicker.detach();

  printf("\n>>> Changing Mode <<<\n");
  if (currentMode == TEST) {
    printf("Switching from TEST to NORMAL mode...\n");
    newMode = NORMAL;
    threadMode = 1;
    measurementTicker.attach(&measurementTickerCallback, NORMAL_INTERVAL);
    averageTicker.attach(&averageTickerCallback, AVERAGE_INTERVAL);
    setLEDState(true, true, true);
    led1 = 0;
    led2 = 1;
    led3 = 0;
  } else if (currentMode == NORMAL) {
    printf("Switching from NORMAL to ADVANCED mode...\n");
    newMode = ADVANCED;
    threadMode = 2;
    measurementTicker.attach(&measurementTickerCallback, NORMAL_INTERVAL);
    averageTicker.attach(&averageTickerCallback, AVERAGE_INTERVAL);
    setLEDState(true, true, true);
    led1 = 0;
    led2 = 0;
    led3 = 1;
  } else if (currentMode == ADVANCED) {
    printf("Switching from ADVANCED to TEST mode...\n");
    newMode = TEST;
    threadMode = 0;
    measurementTicker.attach(&measurementTickerCallback, TEST_INTERVAL);
    setLEDState(true, true, true);
    led1 = 1;
    led2 = 0;
    led3 = 0;
  }

  currentMode = newMode;
  changeModeRequest = false;
  printf(">>> Mode Change Complete <<<\n");
}

// ALL MODES: Function to print the current mode to the console for debugging
void printMode() {
  printf("\n========================\n");
  if (currentMode == TEST) {
    printf("CURRENT MODE: TEST\n");
  } else if (currentMode == NORMAL) {
    printf("CURRENT MODE: NORMAL\n");
  } else if (currentMode == ADVANCED) {
    printf("CURRENT MODE: ADVANCED\n");
  }
  printf("========================\n");
}

void printMeasurements() {
  dataMutex.lock();
  float localMoisture = moisture;
  float localBrightness = brightness;
  int localSatelliteCount = satelliteCount;
  float localLatitude = latitude;
  float localLongitude = longitude;
  float localAltitude = altitude;
  char localMeridian = meridian;
  char localParallel = parallel;
  char localMeasurement = measurement;
  char localGpsModuleClock[10];
  memcpy(localGpsModuleClock, gpsModuleClock, sizeof(gpsModuleClock));

  int localClear = clear;
  int localRed = red;
  int localGreen = green;
  int localBlue = blue;

  float localXAxis = xAxis;
  float localYAxis = yAxis;
  float localZAxis = zAxis;

  float localTemperature = temperature;
  float localHumidity = humidity;
  dataMutex.unlock();

  // Now use local variables to print measurements
  printf("\n--- SENSOR MEASUREMENTS ---\n");
  printf("Soil Moisture: %.1f%%\n", localMoisture);
  printf("Light Intensity: %.2f%%\n", localBrightness);
  printf("GPS: Satellites: %d\n", localSatelliteCount);
  printf("     Latitude: %.6f %c, Longitude: %.6f %c, Altitude: %.1f %c\n",
         localLatitude, localParallel, localLongitude, localMeridian,
         localAltitude, localMeasurement);
  printf("Clock: %s\n", localGpsModuleClock);
  printf("Color Sensor: Clear: %d, Red: %d, Green: %d, Blue: %d\n", localClear,
         localRed, localGreen, localBlue);
  printf("Accelerometer: X=%.2f m/s^2, Y=%.2f m/s^2, Z=%.2f m/s^2\n",
         localXAxis, localYAxis, localZAxis);
  printf("Temperature: %.1f°C, Humidity: %.1f%%\n", localTemperature,
         localHumidity);
  printf("--------------------------\n");
}

void printAverages() {
  printf("\n--- NORMAL MODE AVERAGES (Last 10 Seconds) ---\n");

  if (sampleCount > 0) {
    printf("Temperature:\n");
    printf("  Mean: %.2f°C\n", tempSum / sampleCount);
    printf("  Min: %.2f°C, Max: %.2f°C\n", tempMin, tempMax);

    printf("Humidity:\n");
    printf("  Mean: %.2f%%\n", humSum / sampleCount);
    printf("  Min: %.2f%%, Max: %.2f%%\n", humMin, humMax);

    printf("Accelerometer:\n");
    printf("  X: Mean=%.2f, Min=%.2f, Max=%.2f\n", xAxisSum / sampleCount,
           xAxisMin, xAxisMax);
    printf("  Y: Mean=%.2f, Min=%.2f, Max=%.2f\n", yAxisSum / sampleCount,
           yAxisMin, yAxisMax);
    printf("  Z: Mean=%.2f, Min=%.2f, Max=%.2f\n", zAxisSum / sampleCount,
           zAxisMin, zAxisMax);

    printf("Light Intensity:\n");
    printf("  Mean: %.2f%%\n", lightSum / sampleCount);
    printf("  Min: %.2f%%, Max: %.2f%%\n", lightMin, lightMax);

    printf("Soil Moisture:\n");
    printf("  Mean: %.2f%%\n", soilSum / sampleCount);
    printf("  Min: %.2f%%, Max: %.2f%%\n", soilMin, soilMax);

    printDominantColorSummary();
  } else {
    printf("No data collected during this interval.\n");
  }

  printf("---------------------------------------------\n");
}

// NORMAL MODE
void printDominantColorSummary() {
  printf("\nColor Sensor: Dominant Colors:\n");
  printf("Red: %d, Green: %d, Blue: %d\n", redCount, greenCount, blueCount);

  if (redCount > greenCount && redCount > blueCount) {
    printf("Overall Dominant Color: RED\n");
  } else if (greenCount > redCount && greenCount > blueCount) {
    printf("Overall Dominant Color: GREEN\n");
  } else if (blueCount > redCount && blueCount > greenCount) {
    printf("Overall Dominant Color: BLUE\n");
  } else {
    printf("No Clear Dominant Color\n");
  }

  redCount = greenCount = blueCount = 0; // Reset counts
}

void updateStats(float temp, float hum, float x, float y, float z, float light,
                 float soil) {
  if (sampleCount == 0) {
    tempMin = tempMax = temp;
    humMin = humMax = hum;
    xAxisMin = xAxisMax = x;
    yAxisMin = yAxisMax = y;
    zAxisMin = zAxisMax = z;
    lightMin = lightMax = light;
    soilMin = soilMax = soil;
  }

  // Update sums
  tempSum += temp;
  humSum += hum;
  xAxisSum += x;
  yAxisSum += y;
  zAxisSum += z;
  lightSum += light;
  soilSum += soil;

  // Update min/max
  if (temp < tempMin)
    tempMin = temp;
  if (temp > tempMax)
    tempMax = temp;
  if (hum < humMin)
    humMin = hum;
  if (hum > humMax)
    humMax = hum;
  if (x < xAxisMin)
    xAxisMin = x;
  if (x > xAxisMax)
    xAxisMax = x;
  if (y < yAxisMin)
    yAxisMin = y;
  if (y > yAxisMax)
    yAxisMax = y;
  if (z < zAxisMin)
    zAxisMin = z;
  if (z > zAxisMax)
    zAxisMax = z;
  if (light < lightMin)
    lightMin = light;
  if (light > lightMax)
    lightMax = light;
  if (soil < soilMin)
    soilMin = soil;
  if (soil > soilMax)
    soilMax = soil;

  sampleCount++;
}

void resetStats() {
  tempSum = humSum = xAxisSum = yAxisSum = zAxisSum = 0;
  lightSum = soilSum = 0;
  tempMin = humMin = xAxisMin = yAxisMin = zAxisMin = 0;
  lightMin = soilMin = 0;
  tempMax = humMax = xAxisMax = yAxisMax = zAxisMax = 0;
  lightMax = soilMax = 0;
  sampleCount = 0;
}
