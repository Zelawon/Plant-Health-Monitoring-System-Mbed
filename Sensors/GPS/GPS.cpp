#include "GPS.h"
#include <cstdlib>
#include <cstring>

// Constructor to initialize GPS communication and buffer
GPS::GPS(PinName tx, PinName rx, int baud_rate)
    : gps_serial(tx, rx, baud_rate) {
  memset(buffer, 0, sizeof(buffer)); // Initialize buffer with zeros
}

// Convert NMEA format coordinates to decimal degrees
float GPS::convertToDecimalDegrees(float nmeaCoord) {
  int degrees = static_cast<int>(nmeaCoord) / 100; // Get degrees
  float minutes =
      nmeaCoord - (degrees * 100);    // Get minutes by removing degrees
  return degrees + (minutes / 60.0f); // Convert
}
// Global variables for storing last known latitude and longitude
float last_latitude = 0.0f;
float last_longitude = 0.0f;

// Read GPS data from the buffer and parse if valid GPGGA sentence is found
void GPS::readData(int &num_satellites, float &latitude, float &longitude,
                   char &meridian, char &parallel, float &altitude,
                   char &measurement, char gps_time[10]) {

  memset(buffer, 0, sizeof(buffer)); // Clear buffer for fresh read

  // Check if data is available in GPS serial buffer
  if (gps_serial.readable()) {
    int bytesRead =
        gps_serial.read(buffer, sizeof(buffer) - 1); // Read data to buffer
    if (bytesRead > 0) {
      buffer[bytesRead] = '\0'; // Null-terminate the buffer

      // Locate GPGGA sentence in the buffer
      char *start = strstr(buffer, "$GPGGA");
      if (start != nullptr) {
        // Parse GPS data from the GPGGA sentence
        parseData(start, num_satellites, latitude, longitude, meridian,
                  parallel, altitude, measurement, gps_time);

        // Log if location data has changed
        if (latitude != last_latitude || longitude != last_longitude) {
          printf("GPS Update - Latitude: %.6f, Longitude: %.6f\n", latitude,
                 longitude);
          last_latitude = latitude;
          last_longitude = longitude;
        }
      }
    }
  }
}

// Parse NMEA GPGGA sentence for GPS information and update references
void GPS::parseData(const char *nmea_sentence, int &num_satellites,
                    float &latitude, float &longitude, char &meridian,
                    char &parallel, float &altitude, char &measurement,
                    char gps_time[10]) {

  // Ensure sentence is a GPGGA sentence
  if (strncmp(nmea_sentence, "$GPGGA", 6) == 0) {
    int fieldIndex = 0;  // Track the current field in the sentence
    int i = 0, j = 0;    // Indices for parsing fields
    char field[16] = ""; // Buffer to hold each field
    int hours = 0, minutes = 0, seconds = 0;
    // Loop through characters in the sentence
    while (nmea_sentence[i] != '\0') {

      if (nmea_sentence[i] == ',' || nmea_sentence[i + 1] == '\0') {
        field[j] = '\0'; // Null-terminate the field

        // Process each field based on its index
        switch (fieldIndex) {
        case 1:         // Time field (HHMMSS format)
          if (j >= 6) { // Ensure the field contains at least HHMMSS
            hours = (field[0] - '0') * 10 + (field[1] - '0'); // Extract hours
            minutes =
                (field[2] - '0') * 10 + (field[3] - '0'); // Extract minutes
            seconds =
                (field[4] - '0') * 10 + (field[5] - '0'); // Extract seconds

            // Apply timezone offset for Spain (UTC+1 or UTC+2 for DST)
            hours += 1; // Adjust for standard time
            if (hours >= 24) {
              hours -= 24; // Wrap around if past midnight
            }

            // Format corrected time back into gps_time
            snprintf(gps_time, 10, "%.2d:%.2d:%.2d", hours, minutes, seconds);
          }
          break;
        case 2:
          latitude = convertToDecimalDegrees(atof(field));
          break;
        case 3:
          parallel = field[0];
          break;
        case 4:
          longitude = convertToDecimalDegrees(atof(field));
          break;
        case 5:
          meridian = field[0];
          break;
        case 7:
          num_satellites = atoi(field);
          break;
        case 9:
          altitude = atof(field);
          break;
        case 10:
          measurement = field[0] + 32;
          break; // Lowercase conversion for unit
        }

        j = 0;        // Reset field index
        fieldIndex++; // Move to the next field
      } else {
        field[j++] = nmea_sentence[i]; // Append character to field buffer
      }
      i++;
    }

    // Apply direction to latitude and longitude (S and W make negative)
    if (parallel == 'S')
      latitude = -latitude;
    if (meridian == 'W')
      longitude = -longitude;
  }
}
