#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "gps_helpers.h"
#include <Adafruit_PN532.h>
#include <U8g2lib.h>



//#define ST_USE_ST7789_DISPLAY
#define ST_USE_SSD1306_DISPLAY

#if defined(ST_USE_SSD1306_DISPLAY)
  //
  // SSD1306 Definitions
  //
  #define DISPLAY_SCREEN_WIDTH          128 // OLED display width, in pixels
  #define DISPLAY_SCREEN_HEIGHT         64 // OLED display height, in pixels
  #define DISPLAY_OLED_RESET            -1 // Reset pin # (or -1 for shared reset pin)
  #define DISPLAY_SCREEN_ADDRESS        0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
#elif defined(ST_USE_ST7789_DISPLAY)
  //
  // ST7789 Definitions
  //
  #define DISPLAY_SCREEN_WIDTH          240 // OLED display width, in pixels
  #define DISPLAY_SCREEN_HEIGHT         320 // OLED display height, in pixels

  //
  // Background and text font color definitions
  //
  #define ST_DISPLAY_BG_COLOR           ST77XX_BLACK
  #define ST_DISPLAY_TEXT_COLOR         ST77XX_YELLOW

  //
  // SD77XX Definitions
  //
  #define TFT_CS      15
  #define TFT_DC      2
  #define TFT_RST     4
  #define TFT_MOSI    23
  #define TFT_SCLK    18
  #define TFT_MISO    19
#endif
//
// Touch Sensor definitions
//
#define BUTTON_INPUT_PIN              12 // GPIO based pin number of the touch sensor (interrupt based trigger)

//
// Status LED definitions
//
#define LED_GREEN_OUTPUT_PIN          33 // GPIO based pin number of the LED that is illuminated 
                                        

//
// NFC PIN Definitions
//
#define PN532_IRQ   (-1) // Unused for the ESP32
#define PN532_RESET (-1) // Unused for the ESP32

//
// PN532 Globals
//
Adafruit_PN532 nfc(1, -1, &Wire);

//
// Speed tracking related globals
//
volatile bool touchDetected = false; // Flag that is set when touch sensor is activated
volatile bool speed_tracking_active = false; // Flag to indicate that the location and speed are being tracked
const unsigned long debounceDelay = 100; // Debounce delay in milliseconds
volatile unsigned long lastDebounceTime = 0; // Stores the last time the button was pressed

/*
Finish line bounding box

Far Left:
Latitude: 43.7888582
Longitude: -114.4830335

Near Left:
Latitude: 43.7888427
Longitude: -114.4830817

Far Right:
Latitude: 43.7887691
Longitude: -114.4829758

Near Right:
Latitude: 43.7887540
Longitude: -114.4830180
*/

//
// Race information data structure
//
typedef struct _RACE_INFORMATION {
  int race_id;
  int driver_id;
  int car_id;
  int race_month;
  int race_day;
  int race_year;
} RACE_INFORMATION;

RACE_INFORMATION raceInformation;

//
// Speed tracking data structure definition
//
typedef struct _SPEEDTRACKER_INFO {
  double latitude;
  double longitude;
  double mph;  
} SPEEDTRACKER_INFO;

//
// Maximum number of SPEEDTRACKER_INFO objects in the SPEEDTRACKER_INFO array.
//
#define SPEEDTRACKER_INFO_MAX_ENTRIES   4000

//
// The array of SPEEDTRACKER_INFO objects used to track the speed/location info for a race
//
SPEEDTRACKER_INFO stInfo[SPEEDTRACKER_INFO_MAX_ENTRIES]; // Array of the speedtracker info entries

//
// Index of the current entry in the SPEEDTRACKER_INFO object array
//
uint stInfoCurrentIndex = 0; // Current index of the speedtracker array

//
// Initialize the display with the defined pins
//
//Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Define global variables for configuration settings
int config_value_race_id = 1;
int config_value_car_id = 1;
int config_value_driver_id = 1;
int config_value_race_day = 1;
int config_value_race_month = 1;
int config_value_race_year = 2023;
String config_value_driver_name;
String config_value_driver_car;


//
// WiFi related globals
//
const char* ssid = "speedtracker";
const char* password = "sp33dtrack3r";
const char* serverUrl = "http://10.0.0.1:5000/upload";

// const char* ssid = "CenturyLink8135-5G";
// const char* password = "r5zbjepderfjku";
// const char* serverUrl = "http://192.168.0.40:5000/upload";

//
// ISRs
//
void IRAM_ATTR buttonInterrupt() {
  // Check if the debounce delay has passed since the last button press
  if ((millis() - lastDebounceTime) > debounceDelay) {
    touchDetected = true;
    lastDebounceTime = millis(); // Update the last debounce time
  }
}

void drawReadyScreen(String fullName, String carInfo) {
  u8g2.clearBuffer(); // clear the buffer
  
  // Top section
  u8g2.setFont(u8g2_font_ncenB08_tr); // set font size to 8
  u8g2.drawStr(0, 10, fullName.c_str()); // draw full name
  u8g2.drawStr(0, 20, carInfo.c_str()); // draw car info
  
  // Ready section
  u8g2.setFont(u8g2_font_fub30_tr); // set font size to 30
  u8g2.drawStr(0, 55, "Ready"); // draw "Ready"
  
  u8g2.sendBuffer(); // send the buffer to the display
}

void drawMainScreen(String fullName, String carInfo, int speed) {
  u8g2.clearBuffer(); // clear the buffer
  
  // Top section
  u8g2.setFont(u8g2_font_ncenB08_tr); // set font size to 8
  u8g2.drawStr(0, 10, fullName.c_str()); // draw full name
  u8g2.drawStr(0, 20, carInfo.c_str()); // draw car info
  
  // Speed section
  u8g2.setFont(u8g2_font_fub30_tr); // set font size to 30
  char speedStr[4];
  sprintf(speedStr, "%03d", speed); // add leading zeros if needed
  u8g2.drawStr(0, 55, speedStr); // draw speed
  u8g2.setFont(u8g2_font_fub17_tr); // set font size to 30  
  u8g2.drawStr(73, 55, "mph"); // draw "mph" after speed
  
  u8g2.sendBuffer(); // send the buffer to the display
}

void drawSummaryScreen(int topSpeed) {
  u8g2.clearBuffer(); // clear the buffer
  
  // Top section
  u8g2.setFont(u8g2_font_ncenB08_tr); // set font size to 8
  u8g2.drawStr(0, 10, "Top Speed:"); // draw "Top Speed:"
  
  // Speed section
  u8g2.setFont(u8g2_font_fub30_tr); // set font size to 30
  char speedStr[4];
  sprintf(speedStr, "%03d", topSpeed); // add leading zeros if needed
  u8g2.drawStr(0, 55, speedStr); // draw speed
  u8g2.setFont(u8g2_font_fub17_tr); // set font size to 30  
  u8g2.drawStr(73, 55, "mph"); // draw "mph" after speed
  u8g2.sendBuffer(); // send the buffer to the display
  
  delay(10000); // wait for 10 seconds
}
// void connectToWiFi(const char* ssid, const char* password) {

//   // Initialize Wi-Fi
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);

//   unsigned long start = millis();

//   // Wait for the connection to establish
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     display.print(".");
//     display.display();

//     if ((millis() - start) > 30000) {
//       break;
//     }
//   }

//   if (WiFi.status() == WL_CONNECTED) {
//   }
// }

void setup()
{
  Serial.begin(115200);

  Wire.begin();

  // Increase the clock rate of the I2C bus so that a higher update rate can be supported
  Wire.setClock(400000);

  // Initialize the display
  if(!u8g2.begin()) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Set LED outputs
  //pinMode(LED_RED_OUTPUT_PIN, OUTPUT);
  pinMode(LED_GREEN_OUTPUT_PIN, OUTPUT);
  digitalWrite(LED_GREEN_OUTPUT_PIN, LOW);

  // Initialize SD card if it's not already
  if (!SD.begin(5, SPI, 400000, "/sd", 10, false)) {
    Serial.println("Failed to initialize SD card.");
    return;
  } else {
    Serial.println("Initialized SD card.");
  }

  // Get the race config from the config file
  bool raceConfigExists = getRaceConfig();

  // Print the block of text
  nfc.begin();

  // Check for successful initialization of the NFC reader
  if (!nfc.getFirmwareVersion()) {
    Serial.print("Failed to initialize PN532 board. Halting...");
    //while (1); // halt
  }

  nfc.SAMConfig();

  if (myGNSS.begin(Wire, 0x42, defaultMaxWait, false) == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  } else {
    Serial.println("Initialized GPS...");    
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.setNavigationFrequency(5); //Set output to 10 Hz

  uint8_t rate = myGNSS.getNavigationFrequency(); //Get the update rate of this module
  Serial.println("Current update rate: ");
  Serial.println(rate);

  
  // Set the touchPin as an INPUT
  pinMode(BUTTON_INPUT_PIN, INPUT_PULLUP);

  // Attach the interrupt to the touchPin
  // Trigger the interrupt when the pin goes from HIGH to LOW
  attachInterrupt(digitalPinToInterrupt(BUTTON_INPUT_PIN), buttonInterrupt, FALLING);


  if (!raceConfigExists) {
    Serial.println("Please scan driver card...");

    // Read card scanner for driver name
    String driverName = readCardData();

    // Save the driver information to file
  }    
  
  drawReadyScreen(config_value_driver_name, config_value_driver_car);
}

String readCardData() {
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    String cardData;
    uint8_t pageBuffer[4];

    // Read 4 bytes (1 page) at a time
    for (uint8_t page = 4; page < 40; page++) {
      Serial.print("Reading page ");
      Serial.print(page);
      Serial.println();

      if (nfc.ntag2xx_ReadPage(page, pageBuffer)) {
        Serial.print("Data: ");
        for (uint8_t i = 0; i < 4; i++) {
          if (pageBuffer[i] >= 32 && pageBuffer[i] <= 126) { // Check if the byte is a printable ASCII character
            cardData += (char)pageBuffer[i];
          }
        }
      } else {
        Serial.println("Failed to read page data");
      }
    }

    int startIndex = cardData.indexOf('<');
    int endIndex = cardData.indexOf('>');
    if (startIndex >= 0 && endIndex >= 0 && startIndex < endIndex) {
      return cardData.substring(startIndex + 1, endIndex);
    }
  }
  return "";
}




bool getRaceConfig() {
 
   // Open file
  char* configFileName = "/race_config.txt";  

   File configFile = SD.open(configFileName, FILE_READ);
   if (!configFile) {
     // Config file does not exist or cannot be read
     // Use default values for configuration settings
     Serial.printf("Failed to open file %s\n", configFileName);
     return false;
   } else {
     Serial.printf("Opened file %s\n", configFileName);
   }
  
   // Read in config settings from file
   while (configFile.available()) {
     // Read in line from file
     String line = configFile.readStringUntil('\n');

     Serial.println(line);
    
     // Split line at equal sign (=)
     int equalsIndex = line.indexOf('=');
     if (equalsIndex == -1) {
       // Invalid format
       continue;
     }
    
     // Extract key-value pair
     String key = line.substring(0, equalsIndex);
     String value = line.substring(equalsIndex + 1);
   

    /* File Format
      race_id=10093
      car_id=220
      driver_id=883
      race_day=22
      race_month=7
      race_year = 2023
      */


     // Store key-value pair in global variables
     if (key == "race_id") {
       config_value_race_id = value.toInt();
     } else if (key == "car_id") {
       config_value_car_id = value.toInt();
     } else if (key == "driver_id") {
       config_value_driver_id = value.toInt();
     } else if (key == "race_day") {
       config_value_race_day = value.toInt();
     } else if (key == "race_month") {
       config_value_race_month = value.toInt();
     } else if (key == "race_year") {
       config_value_race_year = value.toInt();
     } else if (key == "driver_name") {
       config_value_driver_name = value;
     } else if (key == "driver_car") {
       config_value_driver_car = value;
     } else {
       // Unknown key
     }
   }
  
   // Load race info structure with config values
   raceInformation.race_id = config_value_race_id;
   raceInformation.car_id = config_value_car_id;
   raceInformation.driver_id = config_value_driver_id;
   raceInformation.race_day = config_value_race_day;
   raceInformation.race_month = config_value_race_month;
   raceInformation.race_year = config_value_race_year;
  
   // Close file
   configFile.close();

   return true;
}


int max_speed = 0;

void loop()
{
    // Check if the touch sensor was pressed
  if (touchDetected) {
    touchDetected = false;

    if (speed_tracking_active != false) {
      // Reset globals      
      speed_tracking_active = false;
      digitalWrite(LED_GREEN_OUTPUT_PIN, LOW); 

      drawSummaryScreen(max_speed);
      drawMainScreen(config_value_driver_name, config_value_driver_car, max_speed);

    } else {
      speed_tracking_active = true;    
      digitalWrite(LED_GREEN_OUTPUT_PIN, HIGH);         
    }
    
    delay(100);
  }

  if (speed_tracking_active) {
    drawMainScreen(config_value_driver_name, config_value_driver_car, max_speed);
    delay(300);
  }
}

unsigned long lastMillis;

void displayInfo()
{
  double mph = mmPerSecondToMph(myGNSS.getGroundSpeed()) ;
  double latitude = getLatitudeDegrees();
  double longitude = getLongitudeDegrees();

  if ((millis() - lastMillis) > 1000) {
    lastMillis = millis();

    // Save the current mph and location info and advance the index
    stInfo[stInfoCurrentIndex].latitude = latitude;
    stInfo[stInfoCurrentIndex].longitude = longitude;
    stInfo[stInfoCurrentIndex].mph = mph;

    stInfoCurrentIndex++;

    // Check for overflow
    if (stInfoCurrentIndex >= SPEEDTRACKER_INFO_MAX_ENTRIES) {
      saveSpeedTrackerInfoToSD();
      stInfoCurrentIndex = 0;      
    }
  }

  drawMainScreen(config_value_driver_name, config_value_driver_car, mph);

}

//Function to save the stInfo array to a JSON file on the SD card
void saveSpeedTrackerInfoToSD() {
  
  char* filename = "/run_data.csv";

  char filedata[SPEEDTRACKER_INFO_MAX_ENTRIES * (sizeof(SPEEDTRACKER_INFO) + 3)];

  // Open the file for writing
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.printf("Failed to open file %s for writing.", filename);
    Serial.println();
    return;
  } else {
    Serial.printf("Opened file %s for writing.", filename);
    Serial.println();
  }

  // Write out the high speed
  file.printf("High Speed: %d\n", max_speed);

  // Add stInfo array data to the JSON array
  for (int i = 0; i < SPEEDTRACKER_INFO_MAX_ENTRIES; i++) {
    file.printf("%.6f,%.6f,%.2f\n", stInfo[i].latitude, stInfo[i].longitude, stInfo[i].mph);
  }

  // Close the file
  file.close();

  // Clear the stInfo array by setting all elements to zero
  memset(stInfo, 0, sizeof(stInfo));

  Serial.println("Data saved to SD card successfully.");
}

// void uploadData(char* uploadData) {

//   // Send the POST request
//   HTTPClient http;
//   http.begin(serverUrl);
//   http.addHeader("Content-Type", "application/text");

//   int httpResponseCode = http.POST(uploadData);
  
//   // Check the response code
//   if (httpResponseCode > 0) {
//     String response = http.getString();
//     display.println("Server response: " + response);
//   } else {
//     display.println("Error on sending POST request");
//   }

//   http.end();
// }



// bool InitializeSDCardAndFileSystem() {
//     Serial.begin(115200);
//     if(!SD.begin(5, SPI, 400000, "/sd", 30, false)){
//         Serial.println("Card Mount Failed");
//         return false;
//     }

//     return true;
// }
