#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>

// Finish line bounding box
struct GpsCoordinate {
  double latitude;
  double longitude;
};

// Race information data structure
typedef struct _RUN_INFORMATION {
  String driver_name;
  String driver_car;
  GpsCoordinate finishLine_left;
  GpsCoordinate finishLine_right;
  double bamf_speed;
  double high_speed;
} RUN_INFORMATION;

RUN_INFORMATION runInformation;

String driver_ke;
String driver_car;
double finishLine_left_lat;
double finishLine_left_lng;
double finishLine_right_lat;
double finishLine_right_lng;
double bamf_speed;


String keys[7] = {"driver_name",
                  "driver_car",
                  "finishLine_left_lng",
                  "finishLine_left_lat",
                  "finishLine_right_lng",
                  "finishLine_right_lat",
                  "bamf_speed"};

void updateConfig(String key, String value) {

  String line;

  if (!SD.exists("/config.txt")) {
    Serial.println("/config.txt doesn't exist");
  
    //
    // Create the config file
    //
    File newConfigFile = SD.open("/config.txt", FILE_WRITE);

    if (!newConfigFile) {
      Serial.println("Failed to create /config.txt");
    } else {
      Serial.println("Created /config.txt");
    }

    //
    // Add the key-value to the file
    //
    newConfigFile.println(key + "=" + value);
    newConfigFile.close();

    Serial.printf("Wrote %s to /config.txt\n", key + "=" + value);
  } else {
    File updatedConfigFile = SD.open("/tmp.txt", FILE_WRITE);
    
    if (!updatedConfigFile) {
      Serial.println("Failed to open /tmp.txt");
      return;
    } else {
      Serial.println("Created /tmp.txt");
    }
    
    while(configFile.available()) {
      line = configFile.readStringUntil('\n');
      Serial.printf("Read %s from config file\n", line.c_str());

      if (line.indexOf(key) > 0) {
        String newEntry = key + "=" + value;
        updatedConfigFile.println(newEntry);
        Serial.printf("Overwrote %s with %s", line.c_str(), newEntry.c_str());
      } else {
        updatedConfigFile.println(line);
        Serial.printf("Rewrote %s to config file\n", line.c_str());
      }
    }

    configFile.close();  // close the file after reading
    SD.remove("/config.txt");
    SD.rename("/tmp.txt", "/config.txt");

    Serial.printf("Rena,ed /tmp.txt to /config.txt\n", line.c_str());

    updatedConfigFile.close();

  }

  // Update the corresponding global variable.
  if (key == "driver_name") {
    runInformation.driver_name = value;
  } else if (key == "driver_car") {
    runInformation.driver_car = value;
  } else if (key == "finishLine_left_lng") {
    runInformation.finishLine_left.longitude = value.toDouble();
  } else if (key == "finishLine_left_lat") {
    runInformation.finishLine_left.latitude = value.toDouble();
  } else if (key == "finishLine_right_lng") {
    runInformation.finishLine_right.longitude = value.toDouble();
  } else if (key == "finishLine_right_lat") {
    runInformation.finishLine_right.latitude = value.toDouble();
  } else if (key == "bamf_speed") {
    runInformation.bamf_speed = value.toDouble();
  }
}

bool isAnyKeyMissing() {
  // List of all keys.
  String keys[7] = {"driver_name", "driver_car", "finishLine_left_lng", "finishLine_left_lat", "finishLine_right_lng", "finishLine_right_lat", "bamf_speed"};

  // Open the config file.
  File configFile = SD.open("/config.txt", FILE_READ);
  if (!configFile) {
    Serial.println("Failed to open config file.");
    return false;
  }

  String configContent = configFile.readString();
  
  // Loop over all keys to check if they are present in the config file.
  for (int i = 0; i < 7; i++) {
    if (!configContent.startsWith(keys[i] + "=")) {
      return true;  // A key is missing.
    }
  }

  return false;  // No keys are missing.
}

bool isKeyMissing(String key) {
  // Open the config file.
  File configFile = SD.open("/config.txt", FILE_READ);
  if (!configFile) {
    Serial.println("Failed to open config file.");
    return false;
  }

  String configContent = configFile.readString();
  
  // Check if the key is present in the config file.
  return !configContent.startsWith(key + "=");
}


void setup() {
  Serial.begin(115200);

  if (!SD.begin(5)) {
    Serial.println("Failed to mount file system");
  }

  String driver_key = "driver_name";
  String driver_car_key = "driver_car";
  String finishLine_left_lat_key = "finishLine_left_lat";
  String finishLine_left_lng_key = "finishLine_left_lat";
  String finishLine_right_lat_key = "finishLine_right_lat";
  String finishLine_right_lng_key = "finishLine_right_lng";
  String bamf_speed_key = "bamf_speed";

  // //
  // // Check for missing keys
  // //
  // bool missingKeys = isAnyKeyMissing();

  // if (missingKeys) {
  //   Serial.println("At least one key is missing from the config file");
  // }

  updateConfig(driver_key, "Jonathan Morrison");
  updateConfig(driver_car_key, "Buick Supreme");
  updateConfig(finishLine_left_lat_key, "47.1234567");
  updateConfig(finishLine_left_lng_key, "-122.1234567");
  updateConfig(finishLine_right_lng_key, "47.7654321");
  updateConfig(finishLine_right_lat_key, "-122.7654321");
  updateConfig(bamf_speed_key, "239.76");

  // if (missingKeys) {
  //   Serial.println("At least one key is missing from the config file");
  // }
}

void loop() {

}
