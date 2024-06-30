/*
  Title: WIO-Terminal Lux Meter Display with Calibration
  Author: [Your Name]
  Date: [Today's Date]

  About:
  This program measures the ambient light level using the built-in light sensor 
  of the WIO Terminal and displays the lux value on the WIO-Terminal's LCD screen. 
  The display uses the same font and color settings as a previous clock program 
  for consistency. The user can calibrate the lux reading by pressing the top 
  left button (SW1).

  License: MIT License
  Copyright (c) [Year] [Your Name]

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

#include <TFT_eSPI.h>    // Graphics and display library for WIO Terminal

TFT_eSPI tft = TFT_eSPI();  // Create display object
float calibrationOffset = 0; // Calibration offset for lux readings
float lastLux = -1; // To store the last lux value for comparison

void setup() {
  tft.begin();
  tft.setRotation(3); // Set the display orientation
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2); // Increase the font size
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  pinMode(WIO_LIGHT, INPUT); // Set the light sensor pin to input mode
  pinMode(WIO_KEY_A, INPUT_PULLUP); // Set the SW1 button pin to input mode with pull-up resistor

  // Initial display setup
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Lux:", 160, 70, 4); // Display the label, higher position
}

void loop() {
  if (digitalRead(WIO_KEY_A) == LOW) {
    calibrateSensor(); // Calibrate sensor when SW1 button is pressed
  }
  
  int sensorValue = analogRead(WIO_LIGHT); // Read the light sensor value
  float voltage = sensorValue * (3.3 / 1023.0); // Convert the sensor value to voltage
  float lux = (voltage * 100) + calibrationOffset; // Convert voltage to lux (approximation) and apply calibration offset
  
  if (abs(lux - lastLux) > 0.1) { // Update display only if lux value changes significantly
    // Erase previous lux value by drawing over it with the background color
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    tft.drawFloat(lastLux, 2, 160, 130, 4); // Lower position
    
    // Draw new lux value
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawFloat(lux, 2, 160, 130, 4); // Lower position

    lastLux = lux; // Update lastLux with current lux value
  }
  
  delay(100); // Shorter delay to make the program more responsive
}

// Function to calibrate the light sensor
void calibrateSensor() {
  int sensorValue = analogRead(WIO_LIGHT); // Read the light sensor value
  float voltage = sensorValue * (3.3 / 1023.0); // Convert the sensor value to voltage
  float lux = voltage * 100; // Convert voltage to lux (approximation)
  calibrationOffset = -lux; // Set calibration offset to negate the current lux value
  
  // Display calibration message
  tft.setTextColor(TFT_BLACK, TFT_BLACK); // Clear previous lux value
  tft.drawFloat(lastLux, 2, 160, 130, 4); // Lower position
  tft.setTextColor(TFT_GREEN, TFT_BLACK); // Set color for calibration message
  tft.drawString("Calibrated", 160, 130, 4); // Display calibration message
  delay(1000); // Display the calibration message for 1 second
  
  // Clear calibration message
  tft.setTextColor(TFT_BLACK, TFT_BLACK);
  tft.drawString("Calibrated", 160, 130, 4);
  lastLux = 0; // Reset lastLux to force update
}
