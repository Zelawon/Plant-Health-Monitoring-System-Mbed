#ifndef PWM_THREAD_H
#define PWM_THREAD_H

#include "mbed.h"
#include "../../Sensors/Color-Sensor/ColorSensor.h"
// Include structs
#include "../../Structs/Violation.h"

// Maximum violations tracked
#define MAX_VIOLATIONS 10

class pwmThread {
public:
    // Constructor
    pwmThread(DigitalOut *redLED, DigitalOut *greenLED, DigitalOut *blueLED,
              int *red, int *green, int *blue, int *clear, int *mode, 
              bool *violationsActive, Violation *violations, size_t *violationCount,
              ColorSensor *colorSensor, Mutex *dataMutex);

    // Thread run method
    void run();

    // Pause and resume methods
    void pause();
    void resume();

private:
    DigitalOut *redLED;
    DigitalOut *greenLED;
    DigitalOut *blueLED;

    int *red, *green, *blue, *clear;
    int *mode;
    bool *violationsActive;
    Violation *violations;
    size_t *violationCount;

    ColorSensor *rgbColorSensor;
    Mutex *dataMutex;

    bool _running;

    // Helper methods
    void emulatePWM(DigitalOut &led, float dutyCycle);
    void handleViolations(size_t &currentViolationIndex);
    void displayPWM();
    void clearLEDs();
};

#endif // PWM_THREAD_H
