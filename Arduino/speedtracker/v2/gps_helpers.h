#include <SparkFun_u-blox_GNSS_Arduino_Library.h> 

//
// Global object representing the one and only physical GPS device
//
SFE_UBLOX_GNSS myGNSS;

double mmPerSecondToMph(float mmPerSecond) {
  // Convert mm/s to miles/hour
  double milesPerHour = (mmPerSecond / 1000.0) * 2.23694;
  return milesPerHour;
}

double convertToDegrees(int32_t coordinate) {
  double degrees = coordinate / 10000000.0;
  return degrees;
}

double getLongitudeDegrees(uint16_t maxWait = defaultMaxWait) {
  int32_t longitude = myGNSS.getLongitude(maxWait);
  return convertToDegrees(longitude);
}

double getLatitudeDegrees(uint16_t maxWait = defaultMaxWait) {
  int32_t latitude = myGNSS.getLatitude(maxWait);
  return convertToDegrees(latitude);
}
