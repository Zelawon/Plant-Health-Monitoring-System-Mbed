# Plant Health Monitoring System

This repository contains the source code and documentation for the Plant Health Monitoring System. The system monitors environmental parameters such as temperature, humidity, soil moisture, light intensity, acceleration, and GPS location. It provides real-time feedback through RGB LEDs and terminal outputs, alerting users when conditions exceed predefined thresholds.

---

## Features

- Monitors key environmental parameters:
  - **Temperature and Humidity** (Si7021)
  - **Soil Moisture** (SEN-13322)
  - **Light Intensity** (HW5P-1)
  - **Acceleration** (MMA8451Q)
  - **GPS Location and Time** (Ultimate GPS Breakout)
  - **Leaf Color Detection** (TCS34725)
- RGB LEDs indicate violations for different parameters:
  - **Red, Green, and Blue LEDs**, with combinations for some violations.
- Three operating modes:
  1. **TEST Mode**: Real-time monitoring (2-second intervals).
  2. **NORMAL Mode**: Periodic data collection and violation detection (30-second intervals).
  3. **ADVANCED Mode**: Includes NORMAL mode features plus LED color emulation using PWM.
- Multi-threaded system for concurrent sensor readings and feedback.

---

## Project Structure

```plaintext
├── Sensors/
│   ├── GPS/                  // GPS module
│   ├── Color-Sensor/         // RGB color sensor (TCS34725)
│   ├── Accelerometer/        // Accelerometer sensor (MMA8451Q)
│   ├── Humidity-Temperature/ // Humidity & Temp sensor (Si7021)
│   ├── LightSensor/          // Ambient light sensor
│   ├── MoistureSensor/       // Soil moisture sensor
├── Threads/
│   ├── I2C/                  // Thread to handle I2C sensors
│   ├── IO/                   // Thread for analog sensors & GPS
│   ├── PWM/                  // Thread for RGB LED handling
├── Structs/                  // Violation struct
├── main.cpp                  // Main application logic
└── README.md                 // Project overview and setup guide
```

---

## Setup Instructions

### Hardware Setup

1. **Connect the sensors to the microcontroller**:
   - **I2C Sensors** (Si7021, MMA8451Q, TCS34725):
     - SDA: `PB_9`, SCL: `PB_8`
   - **Analog Sensors**:
     - Light Sensor: `PA_4`
     - Soil Moisture Sensor: `PA_0`
   - **GPS Module**:
     - TX: `PA_9`, RX: `PA_10`
   - **RGB LEDs**:
     - Red: `PB_15`, Green: `PA_12`, Blue: `PA_11`
2. **Ensure all components are powered correctly**.

---

## How to Use

1. **Power on the microcontroller**.
2. **Switch between modes** using the user button:
   - **TEST Mode**: Real-time monitoring (default).
   - **NORMAL Mode**: Periodic data collection and violation detection (press button once).
   - **ADVANCED Mode**: Adds PWM-based RGB LED color emulation (press button twice).
3. **Observe the terminal output** for sensor readings and RGB LEDs for violations:
   - **Red LED**: Temperature or X-axis acceleration violation.
   - **Green LED**: Humidity, Y-axis acceleration, or soil moisture violation.
   - **Blue LED**: Light intensity or Z-axis acceleration violation.
   - **Combinations**:
     - **Green + Blue** for soil moisture.
     - **Red + Green** for Y and Z acceleration violations.

