#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi credentials
const char* ssid     = "SSID";
const char* password = "PASSPHRASE";

// Timezone setting for Chicago
const long gmtOffset_sec = -21600; // GMT offset for Chicago (UTC - 6 hours)
const int daylightOffset_sec = 3600; // Daylight saving time offset

// Create object "tft"
TFT_eSPI tft = TFT_eSPI();

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", gmtOffset_sec + daylightOffset_sec, 60000); // NTP server, time offset in seconds, update interval in milliseconds

int centerX, centerY, radius;
int prevSecX = 0, prevSecY = 0;
int prevMinX = 0, prevMinY = 0;
int prevHourX = 0, prevHourY = 0;

unsigned long lastNtpUpdateTime = 0;
unsigned long lastRetryTime = 0;
const unsigned long ntpUpdateInterval = 5 * 60 * 1000; // 5 minutes
const unsigned long retryInterval = 30 * 1000; // 30 seconds
bool ntpSuccess = false;

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
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize NTP
  timeClient.begin();

  // Draw clock face once
  drawClockFace();
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastNtpUpdateTime >= ntpUpdateInterval || lastNtpUpdateTime == 0) {
    if (WiFi.status() == WL_CONNECTED) {
      tft.fillCircle(centerX, centerY - radius - 20, 5, TFT_YELLOW); // Yellow dot while getting time
      if (timeClient.update()) {
        ntpSuccess = true;
        lastNtpUpdateTime = currentMillis;
        tft.fillCircle(centerX, centerY - radius - 20, 5, TFT_GREEN); // Green dot for successful update
      } else {
        ntpSuccess = false;
        tft.fillCircle(centerX, centerY - radius - 20, 5, TFT_RED); // Red dot for failed update
      }
    } else {
      ntpSuccess = false;
      tft.fillCircle(centerX, centerY - radius - 20, 5, TFT_RED); // Red dot for failed update
    }
  } else if (!ntpSuccess && currentMillis - lastRetryTime >= retryInterval) {
    lastRetryTime = currentMillis;
    if (WiFi.status() == WL_CONNECTED) {
      tft.fillCircle(centerX, centerY - radius - 20, 5, TFT_YELLOW); // Yellow dot while retrying
      if (timeClient.update()) {
        ntpSuccess = true;
        lastNtpUpdateTime = currentMillis;
        tft.fillCircle(centerX, centerY - radius - 20, 5, TFT_GREEN); // Green dot for successful update
      }
    }
  }

  // Extract the time components
  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  int seconds = timeClient.getSeconds();

  // Draw analog clock
  drawAnalogClock(hours, minutes, seconds);

  // Wait for 1 second before updating the screen again
  delay(1000);
}

void drawClockFace() {
  // Draw clock face
  tft.drawCircle(centerX, centerY, radius, TFT_WHITE);

  // Draw hour markers and numbers
  for (int i = 1; i <= 12; i++) {
    float angle = (i - 3) * 30 * PI / 180; // Adjusted angle calculation
    int x1 = centerX + (radius - 10) * cos(angle);
    int y1 = centerY + (radius - 10) * sin(angle);
    int x2 = centerX + radius * cos(angle);
    int y2 = centerY + radius * sin(angle);
    tft.drawLine(x1, y1, x2, y2, TFT_WHITE);

    // Draw hour numbers
    int xNum = centerX + (radius - 20) * cos(angle);
    int yNum = centerY + (radius - 20) * sin(angle);
    String hourStr = String(i);
    int16_t x1Num = xNum - 6; // Approximate centering
    int16_t y1Num = yNum - 7; // Approximate centering
    tft.setCursor(x1Num, y1Num);
    tft.print(hourStr);
  }

  // Draw minute markers
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
  if (prevSecX != 0 || prevSecY != 0) {
    tft.drawLine(centerX, centerY, prevSecX, prevSecY, TFT_BLACK);
    tft.drawLine(centerX, centerY, prevMinX, prevMinY, TFT_BLACK);
    tft.drawLine(centerX, centerY, prevHourX, prevHourY, TFT_BLACK);
  }

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
  prevSecX = centerX + radius * 0.85 * cos((secondAngle - 90) * PI / 180);
  prevSecY = centerY + radius * 0.85 * sin((secondAngle - 90) * PI / 180);
  tft.drawLine(centerX, centerY, prevSecX, prevSecY, TFT_RED);
}