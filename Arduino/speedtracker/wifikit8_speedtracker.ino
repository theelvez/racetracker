#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>

const char* ssid = "duiphone";
const char* password = "papasride";
const int OLED_RESET = 16;
Adafruit_SSD1306 display(OLED_RESET);

const int LED_PIN = D7;

TinyGPSPlus gps;
HardwareSerial SerialGPS(D5, D6);

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  SerialGPS.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi connected!");
  display.display();
}

void loop() {
  // Read GPS data
  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read())) {
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("GPS Configuration:");
      display.print("  GPS lock: ");
      display.println(gps.satellites.value() > 0 ? "yes" : "no");
      display.print("  Date/Time: ");
      display.print(gps.date.day());
      display.print("/");
      display.print(gps.date.month());
      display.print("/");
      display.print(gps.date.year());
      display.print(" ");
      display.print(gps.time.hour());
      display.print(":");
      display.print(gps.time.minute());
      display.print(":");
      display.print(gps.time.second());
      display.println();
      display.display();
    }
  }

  // Blink LED to indicate status
  static unsigned long blinkTime = 0;
  if (millis() - blinkTime >= 1000) {
    blinkTime = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }

  // Wait for GPS lock
  if (gps.satellites.value() == 0) {
    return;
  }

  // Get current GPS coordinates and ground speed
  float lat = gps.location.lat();
  float lon = gps.location.lng();
  float speed = gps.speed.mps();

  // Print GPS data to OLED display
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("GPS Data:");
  display.print("  Latitude: ");
  display.println(lat, 6);
  display.print("  Longitude: ");
  display.println(lon, 6);
  display.print("  Speed: ");
  display.print(speed);
  display.println(" m/s");
  display.display();
}
