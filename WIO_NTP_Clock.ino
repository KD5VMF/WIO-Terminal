/*  
License: MIT License
  Copyright (c) [2024] [Adam Figueroa]

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
 */

#include <TFT_eSPI.h>      // Graphics and font library for ILI9341
#include <rpcWiFi.h>       // Wi-Fi library for WIO Terminal
#include <WiFiUdp.h>       // UDP library required for NTP communication
#include <NTPClient.h>     // NTP Client library for time synchronization
#include <TimeLib.h>       // Time library for handling dates and times

// Wi-Fi network credentials
const char* ssid     = "AP_Name";
const char* password = "Passphrase";

// Timezone offset in seconds (e.g., for CDT: -5 * 3600 seconds)
long timezoneOffset = -5 * 3600;

// NTP server details and update interval (in milliseconds)
WiFiUDP ntpUDP;  // UDP client for NTP communication
NTPClient timeClient(ntpUDP, "pool.ntp.org", timezoneOffset, 60000);

// TFT display initialization
TFT_eSPI tft = TFT_eSPI();

// Variables to store previous time and date for comparison
String prevFormattedTime = "";
String prevFormattedDate = "";

// Timing variables to track NTP updates and second increments
unsigned long lastNTPUpdate = 0;
unsigned long lastSecondUpdate = 0;

// Update intervals in milliseconds
const unsigned long updateInterval60Sec = 60000;       // 60 seconds
const unsigned long updateInterval5Min = 300000;       // 5 minutes
const unsigned long updateInterval10Min = 600000;      // 10 minutes
const unsigned long updateInterval15Min = 900000;      // 15 minutes
const unsigned long updateInterval30Min = 1800000;     // 30 minutes
const unsigned long updateInterval1Hour = 3600000;     // 1 hour
const unsigned long updateInterval2Hour = 7200000;     // 2 hours
const unsigned long updateInterval5Hour = 18000000;    // 5 hours

// Uncomment the desired update interval
// const unsigned long ntpUpdateInterval = updateInterval60Sec;  // 60 seconds
const unsigned long ntpUpdateInterval = updateInterval5Min;  // 5 minutes
// const unsigned long ntpUpdateInterval = updateInterval10Min;  // 10 minutes
// const unsigned long ntpUpdateInterval = updateInterval15Min; // 15 minutes
// const unsigned long ntpUpdateInterval = updateInterval30Min;  // 30 minutes
// const unsigned long ntpUpdateInterval = updateInterval1Hour;  // 1 hour
// const unsigned long ntpUpdateInterval = updateInterval2Hour;  // 2 hours
// const unsigned long ntpUpdateInterval = updateInterval5Hour;  // 5 hours

void setup() {
  Serial.begin(115200); // Start serial communication for debugging

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize and configure the TFT display
  tft.init();
  tft.setRotation(3);  // Rotate display 180 degrees
  tft.fillScreen(TFT_BLACK);  // Clear screen with BLACK color
  tft.setTextDatum(MC_DATUM); // Set text datum to middle center

  // Display initial message in red while looking for the proper time
  tft.setTextSize(2);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString("Looking for proper time...", tft.width() / 2, tft.height() / 2, 4);

  updateFromNTP(); // Perform initial time update from NTP
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Update time from NTP server based on the selected interval
  if (currentMillis - lastNTPUpdate >= ntpUpdateInterval) {
    updateFromNTP();
    lastNTPUpdate = currentMillis; // Reset timing for next NTP update
  } else if (currentMillis - lastSecondUpdate >= 1000) {
    // Increment the fake clock every second
    adjustTime(1);
    lastSecondUpdate = currentMillis; // Reset timing for next second increment
  }

  // Display the current time and date on the TFT screen
  displayTimeAndDate();
}

// Function to update time and date from NTP server
void updateFromNTP() {
  tft.fillScreen(TFT_BLACK); // Clear screen before displaying messages
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Retrieving Time from NTP Server...", tft.width() / 2, tft.height() / 2, 4);

  if (timeClient.update()) {
    setTime(timeClient.getEpochTime());
    tft.fillScreen(TFT_BLACK); // Clear screen after successful update
  } else {
    Serial.println("Failed to retrieve time from NTP server");
    tft.fillScreen(TFT_BLACK); // Clear screen before displaying error message
    tft.drawString("Time failed to be received", tft.width() / 2, tft.height() / 2, 4);
  }
}

// Function to display the current time and date on the TFT screen
void displayTimeAndDate() {
  // Format the current time and date
  String currentFormattedTime = timeClient.getFormattedTime();
  String currentFormattedDate = String(month()) + "/" + String(day()) + "/" + String(year());

  // Update the display if time has changed
  if (currentFormattedTime != prevFormattedTime) {
    tft.setTextSize(3); // Enlarge text size for better visibility
    tft.setTextColor(TFT_GREEN, TFT_BLACK); // Green text with black background
    tft.drawString(currentFormattedTime, tft.width() / 2, tft.height() / 2 - 30, 4);
    prevFormattedTime = currentFormattedTime;
  }

  // Update the display if date has changed
  if (currentFormattedDate != prevFormattedDate) {
    tft.setTextSize(2); // Slightly smaller text for the date
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(currentFormattedDate, tft.width() / 2, tft.height() / 2 + 45, 4);
    prevFormattedDate = currentFormattedDate;
  }
}
