/*
  License: MIT License
  Author: Adam Figueroa
  Year: 2024

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
  
******************************************************************************************************************************************

  Overview:
  This Arduino sketch displays an analog clock on a TFT screen using a Wio Terminal. 
  It fetches the current time from an NTP server over Wi-Fi and adjusts the time 
  based on the selected time zone and Daylight Saving Time (DST) settings. The 
  time zone can be cycled through the top left and top middle buttons, and DST 
  can be toggled using the top right button. The current time zone is displayed 
  at the top left corner of the screen along with the DST status.

  Features:
  - Displays an analog clock on a TFT screen.
  - Fetches the current time from an NTP server.
  - Allows cycling through 11 time zones in the Americas and UTC using two buttons.
  - Toggles Daylight Saving Time (DST) on and off using a third button.
  - Displays the current time zone and DST status on the screen.

  Usage:
  1. Connect the Wio Terminal to a computer and open this sketch in the Arduino IDE.
  2. Enter your Wi-Fi credentials in the 'ssid1', 'password1', 'ssid2', and 'password2' 
     variables to allow the Wio Terminal to connect to the internet.
  3. Upload the sketch to the Wio Terminal.
  4. The Wio Terminal will display an analog clock and connect to the NTP server to fetch 
     the current time.
  5. Use the top left button (WIO_KEY_A) to cycle forward through the time zones.
  6. Use the top middle button (WIO_KEY_B) to cycle backward through the time zones.
  7. Use the top right button (WIO_KEY_C) to toggle DST on and off.
  8. The current time zone and DST status will be displayed in the top left corner of the screen.

  Buttons:
  - Top left button (WIO_KEY_A): Cycle forward through the time zones.
  - Top middle button (WIO_KEY_B): Cycle backward through the time zones.
  - Top right button (WIO_KEY_C): Toggle Daylight Saving Time (DST) on and off.

  Dependencies:
  - TFT_eSPI library for the TFT display.
  - WiFi library for Wi-Fi connectivity.
  - NTPClient library for fetching time from an NTP server.
  - Seeed_Arduino_FS and Seeed_SD libraries for file system and SD card support.

  The time zones included in this sketch are:
  - UTC (Coordinated Universal Time, UTC+0)
  - EST (Eastern Standard Time, UTC-5)
  - CST (Central Standard Time, UTC-6)
  - MST (Mountain Standard Time, UTC-7)
  - PST (Pacific Standard Time, UTC-8)
  - AKST (Alaska Standard Time, UTC-9)
  - HAST (Hawaii-Aleutian Standard Time, UTC-10)
  - AST (Atlantic Standard Time, UTC-4)
  - ART (Argentina Time, UTC-3)
  - NST (Newfoundland Standard Time, UTC-3:30)
  - BRT (Brasilia Time, UTC-2)

  Note:
  - The NTP client updates the time every 5 minutes.
  - The sketch handles reconnection attempts if the Wi-Fi connection is lost.
  - The analog clock is updated every second to reflect the current time.
*/

#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Seeed_Arduino_FS.h>
#include <SD/Seeed_SD.h>

// Wi-Fi credentials
const char* ssid1 = "SSID1";
const char* password1 = "PASSWORD1";
const char* ssid2 = "SSID2";
const char* password2 = "PASSWORD2";

// Timezone settings for the top 11 time zones in the Americas and UTC (offsets in seconds from UTC)
const long timeZones[] = {
  0,       // Coordinated Universal Time (UTC) UTC+0
  -18000,  // Eastern Standard Time (EST) UTC-5
  -21600,  // Central Standard Time (CST) UTC-6
  -25200,  // Mountain Standard Time (MST) UTC-7
  -28800,  // Pacific Standard Time (PST) UTC-8
  -32400,  // Alaska Standard Time (AKST) UTC-9
  -36000,  // Hawaii-Aleutian Standard Time (HAST) UTC-10
  -14400,  // Atlantic Standard Time (AST) UTC-4
  -10800,  // Argentina Time (ART) UTC-3
  -12600,  // Newfoundland Standard Time (NST) UTC-3:30
  -7200    // Brasilia Time (BRT) UTC-2
};

const char* timeZoneNames[] = {
  "UTC", "EST", "CST", "MST", "PST", "AKST", "HAST", "AST", "ART", "NST", "BRT"
};

int currentTimeZoneIndex = 2; // Default to CST
bool isDSTEnabled = true; // DST is enabled by default

TFT_eSPI tft = TFT_eSPI();
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // NTP server, time offset in seconds, update interval in milliseconds

int centerX, centerY, radius;
float prevSecX = 0, prevSecY = 0;
float prevMinX = 0, prevMinY = 0;
float prevHourX = 0, prevHourY = 0;

unsigned long lastNtpUpdateTime = 0;
unsigned long lastRetryTime = 0;
const unsigned long ntpUpdateInterval = 5 * 60 * 1000; // 5 minutes
const unsigned long retryInterval = 30 * 1000; // 30 seconds
bool ntpSuccess = false;
String currentSSID = "";

const int buttonA = WIO_KEY_A; // Top left button
const int buttonB = WIO_KEY_B; // Top middle button
const int buttonC = WIO_KEY_C; // Top right button
unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 150; // Slightly increased debounce delay

void drawTimeZone(bool clearPrevious = false);
void connectToWiFi();
void updateNTPClient();

void setup() {
  // Initialize the TFT screen
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Set center and radius
  centerX = tft.width() / 2;
  centerY = tft.height() / 2;
  radius = min(centerX, centerY) - 10;

  // Connect to Wi-Fi
  Serial.begin(115200);
  connectToWiFi();

  // Initialize NTP
  updateNTPClient();
  timeClient.begin();

  // Draw clock face once
  drawClockFace();

  // Initialize buttons
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  pinMode(buttonC, INPUT_PULLUP);

  // Draw initial time zone
  drawTimeZone(true);
}

void loop() {
  unsigned long currentMillis = millis();

  // Handle button presses
  if (currentMillis - lastButtonPressTime > debounceDelay) {
    if (digitalRead(buttonA) == LOW) {
      currentTimeZoneIndex = (currentTimeZoneIndex + 1) % 11;
      updateNTPClient();
      drawTimeZone(true);
      lastButtonPressTime = currentMillis;
    }
    if (digitalRead(buttonB) == LOW) {
      currentTimeZoneIndex = (currentTimeZoneIndex + 10) % 11; // Go backwards
      updateNTPClient();
      drawTimeZone(true);
      lastButtonPressTime = currentMillis;
    }
    if (digitalRead(buttonC) == LOW) {
      isDSTEnabled = !isDSTEnabled;
      updateNTPClient();
      drawTimeZone(true);
      lastButtonPressTime = currentMillis;
    }
  }

  if (currentMillis - lastNtpUpdateTime >= ntpUpdateInterval || lastNtpUpdateTime == 0) {
    if (WiFi.status() == WL_CONNECTED) {
      tft.fillCircle(tft.width() - 20, 20, 10, TFT_YELLOW); // Yellow dot while getting time
      if (timeClient.update()) {
        ntpSuccess = true;
        lastNtpUpdateTime = currentMillis;
        tft.fillCircle(tft.width() - 20, 20, 10, TFT_GREEN); // Green dot for successful update
      } else {
        ntpSuccess = false;
        tft.fillCircle(tft.width() - 20, 20, 10, TFT_RED); // Red dot for failed update
      }
    } else {
      ntpSuccess = false;
      tft.fillCircle(tft.width() - 20, 20, 10, TFT_RED); // Red dot for failed update
    }
  } else if (!ntpSuccess && currentMillis - lastRetryTime >= retryInterval) {
    lastRetryTime = currentMillis;
    if (WiFi.status() == WL_CONNECTED) {
      tft.fillCircle(tft.width() - 20, 20, 10, TFT_YELLOW); // Yellow dot while retrying
      if (timeClient.update()) {
        ntpSuccess = true;
        lastNtpUpdateTime = currentMillis;
        tft.fillCircle(tft.width() - 20, 20, 10, TFT_GREEN); // Green dot for successful update
      } else {
        tft.fillCircle(tft.width() - 20, 20, 10, TFT_RED); // Red dot for failed update
      }
    } else {
      tft.fillCircle(tft.width() - 20, 20, 10, TFT_RED); // Red dot for failed update
    }
  }

  // Extract the time components
  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  int seconds = timeClient.getSeconds();

  // Draw analog clock
  drawAnalogClock(hours, minutes, seconds);

  // Wait for a short time before updating the screen again
  delay(200); // Slightly increased delay for button responsiveness
}

void drawClockFace() {
  // Draw clock face
  tft.drawCircle(centerX, centerY, radius, TFT_WHITE);

  // Draw hour markers and numbers
  tft.setTextSize(2); // Set larger text size
  for (int i = 1; i <= 12; i++) {
    float angle = (i - 3) * 30 * PI / 180; // Adjusted angle calculation
    int x1 = centerX + (radius - 10) * cos(angle);
    int y1 = centerY + (radius - 10) * sin(angle);
    int x2 = centerX + radius * cos(angle);
    int y2 = centerY + radius * sin(angle);
    tft.drawLine(x1, y1, x2, y2, TFT_WHITE);

    // Draw hour numbers
    int xNum = centerX + (radius - 30) * cos(angle); // Adjust position to accommodate larger text
    int yNum = centerY + (radius - 30) * sin(angle);
    String hourStr = String(i);
    int16_t x1Num = xNum - (tft.textWidth(hourStr) / 2); // Proper centering for larger text
    int16_t y1Num = yNum - (tft.fontHeight() / 2); // Proper centering for larger text
    tft.setCursor(x1Num, y1Num);
    tft.print(hourStr);
  }

  // Draw minute markers
  tft.setTextSize(1); // Reset text size for minute markers
  for (int i = 0; i < 60; i++) {
    if (i % 5 != 0) { // Skip where hour markers are
      float angle = (i - 15) * 6 * PI / 180; // Adjusted angle calculation
      int x1 = centerX + (radius - 5) * cos(angle);
      int y1 = centerY + (radius - 5) * sin(angle);
      int x2 = centerX + radius * cos(angle);
      int y2 = centerY + radius * sin(angle);
      tft.drawLine(x1, y1, x2, y2, TFT_WHITE);
    }
  }
}

void drawAnalogClock(int hours, int minutes, int seconds) {
  // Erase the previous hands by drawing them in black
  tft.drawLine(centerX, centerY, prevSecX, prevSecY, TFT_BLACK);
  tft.drawLine(centerX, centerY, prevMinX, prevMinY, TFT_BLACK);
  tft.drawLine(centerX, centerY, prevHourX, prevHourY, TFT_BLACK);

  // Calculate angles
  float hourAngle = ((hours % 12) + minutes / 60.0) * 30;
  float minuteAngle = (minutes + seconds / 60.0) * 6;
  float secondAngle = seconds * 6;

  // Draw hour hand
  prevHourX = centerX + radius * 0.5 * cos((hourAngle - 90) * PI / 180);
  prevHourY = centerY + radius * 0.5 * sin((hourAngle - 90) * PI / 180);
  tft.drawLine(centerX, centerY, prevHourX, prevHourY, TFT_WHITE);

  // Draw minute hand
  prevMinX = centerX + radius * 0.75 * cos((minuteAngle - 90) * PI / 180);
  prevMinY = centerY + radius * 0.75 * sin((minuteAngle - 90) * PI / 180);
  tft.drawLine(centerX, centerY, prevMinX, prevMinY, TFT_WHITE);

  // Draw second hand
  prevSecX = centerX + radius * 0.70 * cos((secondAngle - 90) * PI / 180); // Shorter than minute hand
  prevSecY = centerY + radius * 0.70 * sin((secondAngle - 90) * PI / 180); // Shorter than minute hand
  tft.drawLine(centerX, centerY, prevSecX, prevSecY, TFT_RED);

  // Redraw hour numbers to ensure they are on top
  drawHourNumbers();
}

void drawHourNumbers() {
  tft.setTextSize(2); // Set larger text size
  for (int i = 1; i <= 12; i++) {
    float angle = (i - 3) * 30 * PI / 180; // Adjusted angle calculation
    int xNum = centerX + (radius - 30) * cos(angle); // Adjust position to accommodate larger text
    int yNum = centerY + (radius - 30) * sin(angle);
    String hourStr = String(i);
    int16_t x1Num = xNum - (tft.textWidth(hourStr) / 2); // Proper centering for larger text
    int16_t y1Num = yNum - (tft.fontHeight() / 2); // Proper centering for larger text
    tft.setCursor(x1Num, y1Num);
    tft.print(hourStr);
  }
}

void drawTimeZone(bool clearPrevious) {
  tft.setTextSize(2); // Increased text size for time zone display
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (clearPrevious) {
    tft.fillRect(0, 0, 80, 40, TFT_BLACK); // Clear previous time zone
    tft.fillRect(tft.width() - 80, tft.height() - 20, 80, 20, TFT_BLACK); // Clear previous DST status
  }
  tft.setCursor(5, 5); // Adjusted position for larger text
  tft.print(timeZoneNames[currentTimeZoneIndex]); // Display current time zone

  // Display DST status
  tft.setTextSize(1); // Smaller text size for DST status
  tft.setCursor(tft.width() - 75, tft.height() - 15); // Position in the lower right corner
  tft.print(isDSTEnabled ? "DST: ON" : "DST: OFF"); // Display DST status
}

void connectToWiFi() {
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (attempt % 2 == 0) {
      Serial.print("Attempting to connect to primary Wi-Fi...");
      WiFi.begin(ssid1, password1);
      currentSSID = ssid1;
    } else {
      Serial.print("Attempting to connect to secondary Wi-Fi...");
      WiFi.begin(ssid2, password2);
      currentSSID = ssid2;
    }

    int waitTime = 0;
    while (WiFi.status() != WL_CONNECTED && waitTime < 10000) {
      delay(500);
      Serial.print(".");
      waitTime += 500;
    }
    Serial.println();

    attempt++;
  }

  Serial.println("Connected to Wi-Fi");
  displaySSID();
}

void displaySSID() {
  tft.setTextSize(1); // Set smaller text size for SSID display
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.fillRect(0, tft.height() - 20, tft.width(), 20, TFT_BLACK); // Clear previous SSID
  tft.setCursor(5, tft.height() - 20); // Position cursor
  tft.print(currentSSID); // Display current SSID
}

void updateNTPClient() {
  int offset = timeZones[currentTimeZoneIndex];
  if (isDSTEnabled) {
    offset += 3600; // Add one hour for DST
  }
  timeClient.setTimeOffset(offset);
}
