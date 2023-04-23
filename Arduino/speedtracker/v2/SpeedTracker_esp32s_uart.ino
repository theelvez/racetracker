#include <SPI.h>
#include <SD.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Wire.h>
#include "gps_helpers.h"
#include <Adafruit_PN532.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#define ST_USE_ST7789_DISPLAY

#if defined(ST_USE_SSD1306_DISPLAY)
  //
  // SSD1306 Definitions
  //
  #define DISPLAY_SCREEN_WIDTH          128 // OLED display width, in pixels
  #define DISPLAY_SCREEN_HEIGHT         64 // OLED display height, in pixels
  #define DISPLAY_OLED_RESET            -1 // Reset pin # (or -1 for shared reset pin)
  #define DISPLAY_SCREEN_ADDRESS        0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#elif defined(ST_USE_ST7789_DISPLAY)
  //
  // ST7789 Definitions
  //
  #define DISPLAY_SCREEN_WIDTH          240 // OLED display width, in pixels
  #define DISPLAY_SCREEN_HEIGHT         320 // OLED display height, in pixels

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
#define TOUCH_SENSOR_INPUT_PIN        12 // GPIO based pin number of the touch sensor (interrupt based trigger)

//
// Status LED definitions
//
#define LED_GREEN_OUTPUT_PIN          32 // GPIO based pin number of the LED that is illuminated 
                                         // while speed tracking is active

#define LED_RED_OUTPUT_PIN            33 // GPIO based pin number of the LED that is illuminated 
                                         // while speed tracking is active

#define LED_BASE_OUTPUT_PIN           34 // GPIO based pin number of the LED that is illuminated 
                                         // while speed tracking is active

#define LED_BLUE_OUTPUT_PIN           35 // GPIO based pin number of the LED that is illuminated 
                                         // while speed tracking is active
                                         

//
// NFC PIN Definitions
//
#define PN532_IRQ   (-1) // Unused for the ESP32
#define PN532_RESET (-1) // Unused for the ESP32

//
// PN532 Globals
//
Adafruit_PN532 nfc(-1, -1, &Wire);

//
// Speed tracking related globals
//
volatile bool touchDetected = false; // Flag that is set when touch sensor is activated
volatile bool speed_tracking_active = false; // Flag to indicate that the location and speed are being tracked

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
#define SPEEDTRACKER_INFO_MAX_ENTRIES   3000

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
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


//
// Background and text font color definitions
//
#define ST_DISPLAY_BG_COLOR           ST77XX_BLACK
#define ST_DISPLAY_TEXT_COLOR         ST77XX_YELLOW

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
void touchISR() {
  touchDetected = true;
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

  Wire.begin(21, 22);

  // Increase the clock rate of the I2C bus so that a higher update rate can be supported
  Wire.setClock(400000);

  // Initialize the display
  //tft.init(240, 320, SPI_MODE3);
  tft.init(DISPLAY_SCREEN_WIDTH, DISPLAY_SCREEN_HEIGHT);

  // Set the rotation (0, 1, 2, or 3)
  tft.setRotation(1);

  // Fill the screen with a white background
  tft.fillScreen(ST_DISPLAY_BG_COLOR);

  //tft.setFont(&FreeMonoBold9pt7b);
  tft.setTextSize(2);
  
  // Set the text color to black
  tft.setTextColor(ST_DISPLAY_TEXT_COLOR);

  // Set the cursor position (x, y)
  tft.setCursor(0, 0);

  // Set LED outputs
  pinMode(LED_RED_OUTPUT_PIN, OUTPUT);
  pinMode(LED_GREEN_OUTPUT_PIN, OUTPUT);
  pinMode(LED_BLUE_OUTPUT_PIN, OUTPUT);
  pinMode(LED_BASE_OUTPUT_PIN, OUTPUT);
  
  digitalWrite(LED_BASE_OUTPUT_PIN, HIGH);
  digitalWrite(LED_BLUE_OUTPUT_PIN, HIGH);
  digitalWrite(LED_RED_OUTPUT_PIN, HIGH);
  digitalWrite(LED_GREEN_OUTPUT_PIN, HIGH);

  // Turn on the RED LED
  digitalWrite(LED_GREEN_OUTPUT_PIN, LOW);
  delay(2000);
  digitalWrite(LED_GREEN_OUTPUT_PIN, HIGH);
  digitalWrite(LED_BLUE_OUTPUT_PIN, LOW);
  delay(2000);
  digitalWrite(LED_BLUE_OUTPUT_PIN, HIGH);
  digitalWrite(LED_RED_OUTPUT_PIN, LOW);

  tft.println("Initializing SD card...");

  // Initialize SD card if it's not already
  if (!SD.begin(/*SD_CS, SPI, 400000, "/sd", 10, false*/)) {
    Serial.println("Failed to initialize SD card.");
    return;
  } else {
    Serial.println("Initialized SD card.");
    tft.println("Initialized SD card...");
  }

  // Get the race config from the config file
  getRaceConfig();

  // Print the block of text
  tft.println("Initializing PN532 NFC Reader...");
  nfc.begin();

  // Check for successful initialization of the NFC reader
  if (!nfc.getFirmwareVersion()) {
    Serial.print("Failed to initialize PN532 board. Halting...");
    //while (1); // halt
  }

  tft.println("Initialized PN532 NFC Reader...");


  tft.println("Initializing GPS...");

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  } else {
    tft.println("Initialized GPS...");    
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.setNavigationFrequency(5); //Set output to 10 Hz

  uint8_t rate = myGNSS.getNavigationFrequency(); //Get the update rate of this module
  tft.println("Current update rate: ");
  tft.println(rate);
  
  // Set the touchPin as an INPUT
  pinMode(TOUCH_SENSOR_INPUT_PIN, INPUT_PULLDOWN);

  // Attach the interrupt to the touchPin
  // Trigger the interrupt when the pin goes from HIGH to LOW
  attachInterrupt(digitalPinToInterrupt(TOUCH_SENSOR_INPUT_PIN), touchISR, FALLING);

  tft.setCursor(0, 0);
  tft.fillScreen(ST_DISPLAY_BG_COLOR);
  tft.println("Please scan driver card...");
  Serial.println("Please scan driver card...");

  // Read card scanner for driver name
  String driverName = readCardData();
  tft.fillScreen(ST_DISPLAY_BG_COLOR);
  tft.println("Driver: ");
  tft.println(driverName);
  Serial.println(driverName);


  delay(5000);

  tft.setCursor(DISPLAY_SCREEN_HEIGHT/2, DISPLAY_SCREEN_WIDTH/2);
  tft.fillScreen(ST_DISPLAY_BG_COLOR);
  tft.println("Ready");
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

// Define global variables for configuration settings
int config_value_race_id = 1;
int config_value_car_id = 1;
int config_value_driver_id = 1;
int config_value_race_day = 1;
int config_value_race_month = 1;
int config_value_race_year = 2023;


void getRaceConfig() {
 
   // Open file
  char* configFileName = "/race_config.txt";  

   File configFile = SD.open(configFileName, FILE_READ);
   if (!configFile) {
     // Config file does not exist or cannot be read
     // Use default values for configuration settings
     Serial.printf("Failed to open file %s\n", configFileName);
     return;
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
}


double max_speed = 0.0;

void loop()
{
    // Check if the touch sensor was pressed
  if (touchDetected) {
    touchDetected = false;
    // Add a debounce delay to avoid 
    // multiple touch events from a single touch

    if (speed_tracking_active != false) {
      // Reset globals      
      speed_tracking_active = false;
      digitalWrite(LED_BLUE_OUTPUT_PIN, HIGH);

      tft.setCursor(0, 0);
      tft.setTextSize(3);
      tft.println("High Speed");
      tft.println(max_speed);

      delay(5000);

      raceInformation.race_day++;
      raceInformation.car_id++;
      raceInformation.driver_id++;
      raceInformation.race_id++;
      raceInformation.race_month++;
      raceInformation.race_year++;

      saveSpeedTrackerInfoToSD();    

      tft.fillScreen(ST_DISPLAY_BG_COLOR);
      tft.println("Ready");
  
    } else {
      speed_tracking_active = true;    
      digitalWrite(LED_BLUE_OUTPUT_PIN, LOW);
      digitalWrite(LED_RED_OUTPUT_PIN, HIGH);
      digitalWrite(LED_GREEN_OUTPUT_PIN, HIGH);

      tft.fillScreen(ST_DISPLAY_BG_COLOR);            
    }
    
    delay(100);
  }

  if (speed_tracking_active) {
    displayInfo();
  }
}

unsigned long lastMillis;

void displayInfo()
{
  tft.setCursor(DISPLAY_SCREEN_WIDTH/2, DISPLAY_SCREEN_HEIGHT/2);

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

    // TODO: Check for overflow
  }

  tft.setCursor(120, 140);
  tft.fillRect(120, 140, 100, 100, ST_DISPLAY_BG_COLOR);

  //if (mph > 2.0) {
    tft.print(mph);
    if (max_speed < mph) {
      max_speed = mph;
    }
  //} else {
  //  display.print(0.00);
  //}    

  tft.println(" Mph");
  tft.print("(");
  tft.print(latitude);
  tft.print("):(");
  tft.print(longitude);
  tft.println(")");
}

//Function to save the stInfo array to a JSON file on the SD card
void saveSpeedTrackerInfoToSD() {
  
  char filename[64];

  // Get the current date and time for the file name
  snprintf(filename, sizeof(filename), "/%02d-%02d-%02d-%02d-%02d-%02d.csv",
           raceInformation.race_id,
           raceInformation.driver_id,
           raceInformation.car_id,
           raceInformation.race_month,
           raceInformation.race_day,
           raceInformation.race_year);

  char filedata[SPEEDTRACKER_INFO_MAX_ENTRIES * (sizeof(SPEEDTRACKER_INFO) + 3)];

  // Open the file for writing
  File file = SD.open(filename, FILE_APPEND);
  if (!file) {
    Serial.printf("Failed to open file %s for writing.", filename);
    Serial.println();
    return;
  } else {
    Serial.printf("Opened file %s for writing.", filename);
    Serial.println();
  }

  // Write out the high speed
  file.printf("High Speed: %.2f\n", max_speed);

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

void uploadData(char* uploadData) {

  // Send the POST request
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/text");

  int httpResponseCode = http.POST(uploadData);
  
  // Check the response code
  if (httpResponseCode > 0) {
    String response = http.getString();
    tft.println("Server response: " + response);
  } else {
    tft.println("Error on sending POST request");
  }

  http.end();
}



bool InitializeSDCardAndFileSystem() {
    Serial.begin(115200);
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return false;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    return true;
}
