Plant Health Monitoring System
This repository contains the source code and documentation for the Plant Health Monitoring System. The system monitors environmental parameters such as temperature, humidity, soil moisture, light intensity, acceleration, and GPS location. It provides real-time feedback through RGB LEDs and terminal outputs, alerting users when conditions exceed predefined thresholds.

Features
Monitors key environmental parameters:
Temperature and Humidity (Si7021)
Soil Moisture (SEN-13322)
Light Intensity (HW5P-1)
Acceleration (MMA8451Q)
GPS Location and Time (Ultimate GPS Breakout)
Leaf Color Detection (TCS34725)
RGB LEDs indicate violations for different parameters:
Red, Green, and Blue LEDs, with combinations for some violations.
Three operating modes:
TEST Mode: Real-time monitoring (2-second intervals).
NORMAL Mode: Periodic data collection and violation detection (30-second intervals).
ADVANCED Mode: Includes NORMAL mode features plus LED color emulation using PWM.
Multi-threaded system for concurrent sensor readings and feedback.

Setup Instructions
Hardware Setup
Connect the sensors to the microcontroller:
I2C Sensors (Si7021, MMA8451Q, TCS34725):
SDA: PB_9, SCL: PB_8
Analog Sensors:
Light Sensor: PA_4
Soil Moisture Sensor: PA_0
GPS Module:
TX: PA_9, RX: PA_10
RGB LEDs:
Red: PB_15, Green: PA_12, Blue: PA_11
Ensure all components are powered correctly.
