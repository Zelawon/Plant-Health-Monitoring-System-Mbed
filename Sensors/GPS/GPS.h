#ifndef GPS_H
#define GPS_H

#include "mbed.h"

class GPS {
public:
    // Constructor initializes GPS serial communication with default baud rate
    GPS(PinName tx, PinName rx, int baud_rate = 9600);

    // Read and parse GPS data from the serial buffer
    void readData(int &num_satellites, float &latitude, float &longitude, char &meridian, 
                  char &parallel, float &altitude, char &measurement, char gps_time[10]);

    // Retrieve the formatted GPS data as a C-string
    const char* getFormattedGPSData() const { return gps_data; }

private:
    // Convert NMEA coordinate format to decimal degrees
    float convertToDecimalDegrees(float nmeaCoord);

    // Parse NMEA sentence to extract GPS information into provided references
    void parseData(const char* nmea_sentence, int &num_satellites, float &latitude, 
                   float &longitude, char &meridian, char &parallel, 
                   float &altitude, char &measurement, char gps_time[10]);

    BufferedSerial gps_serial;   // Serial interface for GPS communication
    int num_satellites;          // Count of satellites used in GPS fix
    float latitude;              // Latitude in decimal degrees
    float longitude;             // Longitude in decimal degrees
    char meridian;               // Longitude direction (E/W)
    char parallel;               // Latitude direction (N/S)
    float altitude;              // Altitude in meters
    char measurement;            // Unit of measurement (e.g., meters for altitude)
    char gps_time[10];           // GPS timestamp in HH:MM:SS format
    char gps_data[256];          // Formatted GPS data string for quick access
    char buffer[256];            // Buffer to temporarily store raw GPS data
};

#endif // GPS_H
