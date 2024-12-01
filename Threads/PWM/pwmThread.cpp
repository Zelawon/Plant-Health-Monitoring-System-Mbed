#include "pwmThread.h"
#include "mbed.h"

#define PWM_FREQUENCY 200.0f     // Frequency in Hz
#define PWM_TARGET_DURATION 0.5f // Target duration in seconds

int loopCount = static_cast<int>(PWM_TARGET_DURATION * PWM_FREQUENCY);

pwmThread::pwmThread(DigitalOut *redLED, DigitalOut *greenLED,
                     DigitalOut *blueLED, int *red, int *green, int *blue,
                     int *clear, int *mode, bool *violationsActive,
                     Violation *violations, size_t *violationCount,
                     ColorSensor *colorSensor, Mutex *dataMutex)
    : redLED(redLED), greenLED(greenLED), blueLED(blueLED), red(red),
      green(green), blue(blue), clear(clear), mode(mode),
      violationsActive(violationsActive), violations(violations),
      violationCount(violationCount), rgbColorSensor(colorSensor),
      dataMutex(dataMutex), _running(true) {}

void pwmThread::run() {
  size_t currentViolationIndex = 0;

  while (true) {
    if (_running) {
      if (*violationsActive && *violationCount > 0 && (*mode == 1 || *mode == 2)) {
        // Handle violations in both NORMAL and ADVANCED modes
        handleViolations(currentViolationIndex);
      } else if (*mode == 2) { // ADVANCED mode
        displayPWM();
      } else {
        // No violations, LEDs OFF
        clearLEDs();
        ThisThread::sleep_for(100ms);
      }
    }
  }
}

void pwmThread::handleViolations(size_t &currentViolationIndex) {
  dataMutex->lock();

  if (*violationCount > 0) {
    // Ensure index is within bounds
    if (currentViolationIndex >= *violationCount) {
      currentViolationIndex = 0;
    }

    // Display current violation
    const Violation &violation = violations[currentViolationIndex];
    *redLED = !violation.red;
    *greenLED = !violation.green;
    *blueLED = !violation.blue;

    // Debug information
    printf("Violation: %s | R=%d, G=%d, B=%d\n", violation.name, violation.red,
           violation.green, violation.blue);

    // Move to the next violation in round-robin
    currentViolationIndex++;
  }

  dataMutex->unlock();

  // Wait before cycling to the next violation
  ThisThread::sleep_for(500ms);
}

void pwmThread::displayPWM() {
  uint16_t localRed, localGreen, localBlue, localClear;

  // Read color sensor data
  rgbColorSensor->readColorData(localClear, localRed, localGreen, localBlue);

  dataMutex->lock();
  *clear = localClear;
  *red = localRed;
  *green = localGreen;
  *blue = localBlue;
  dataMutex->unlock();

  // Scale color data to calculate duty cycle
  float maxColor = std::max({localRed, localGreen, localBlue});
  float scale = (maxColor > 0) ? (255.0f / maxColor) : 1.0f;

  float dutyR =
      (localClear > 0) ? std::min(localRed * scale / localClear, 1.0f) : 0.0f;
  float dutyG =
      (localClear > 0) ? std::min(localGreen * scale / localClear, 1.0f) : 0.0f;
  float dutyB =
      (localClear > 0) ? std::min(localBlue * scale / localClear, 1.0f) : 0.0f;

  printf("Scaled PWM RGB: R=%.2f, G=%.2f, B=%.2f\n", dutyR, dutyG, dutyB);

  // LEDs OFF before starting PWM
  clearLEDs();

  // Emulate PWM
  for (int i = 0; i < loopCount; ++i) {
    emulatePWM(*redLED, dutyR);
    emulatePWM(*greenLED, dutyG);
    emulatePWM(*blueLED, dutyB);
  }

  // LEDs OFF after PWM loop
  clearLEDs();
}

void pwmThread::emulatePWM(DigitalOut &led, float dutyCycle) {
  float period = 1.0f / PWM_FREQUENCY;
  float onTime = dutyCycle * period;
  float offTime = period - onTime;

  if (dutyCycle > 0.0f) {
    led = 0;               // LED ON (active low)
    wait_us(onTime * 1e6); // Wait for ON time
  }
  if (dutyCycle < 1.0f) {
    led = 1;                // LED OFF (active low)
    wait_us(offTime * 1e6); // Wait for OFF time
  }
}

void pwmThread::clearLEDs() {
  *redLED = 1;   // Turn OFF red LED
  *greenLED = 1; // Turn OFF green LED
  *blueLED = 1;  // Turn OFF blue LED
}

void pwmThread::pause() { _running = false; }

void pwmThread::resume() { _running = true; }
