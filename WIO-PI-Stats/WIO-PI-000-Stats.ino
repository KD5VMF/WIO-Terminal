#include <TFT_eSPI.h>
#include <Wire.h>
#include <ctype.h>
#include "Seeed_FS.h"
#include "SD/Seeed_SD.h"  // Include SD card support

// Debug flag to enable/disable console prints
const bool DEBUG = false;  // Set to 'true' to enable debug prints, 'false' to disable

// Initialize TFT display
TFT_eSPI tft = TFT_eSPI();

// I2C Address for the Wio Terminal as a slave
#define I2C_SLAVE_ADDRESS 0x08

// Buffer to accumulate received data
#define MAX_BUFFER_SIZE 26  // Metric name (10 chars) + separator (1 char) + value (15 chars)
char buffer[MAX_BUFFER_SIZE + 1];
int bufferIndex = 0;

// Variables to store previous metric values to prevent unnecessary redraws
String prevMetricValues[8];  // Adjust the size based on the number of metrics displayed
bool labelPrinted[8] = {false};  // Track if labels have been printed

// Color settings
uint16_t backgroundColor;
uint16_t textColor;
const uint16_t colorOptions[] = {
  TFT_BLACK, TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_ORANGE, TFT_PURPLE
};
int bgColorIndex = 4;  // Start with blue background
int textColorIndex = 5;  // Start with yellow text

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(1000); // Allow time for the Serial Monitor to connect

  // Initialize TFT screen
  tft.init();
  tft.setRotation(1);  // Adjust rotation as needed

  // Initialize button pins with pull-up resistors
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  // Initialize SD card
  if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
    if (DEBUG) {
      Serial.println("SD card initialization failed!");
    }
  } else {
    if (DEBUG) {
      Serial.println("SD card initialized.");
    }
    loadColorSettings();
  }

  // Apply initial colors
  backgroundColor = colorOptions[bgColorIndex];
  textColor = colorOptions[textColorIndex];
  tft.fillScreen(backgroundColor);

  tft.setTextSize(2);  // Increased text size to make font bigger

  // Set up I2C as slave
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(receiveData);

  // Draw initial placeholders on the TFT screen
  drawInitialMetrics();

  if (DEBUG) {
    Serial.println("Setup complete, waiting for I2C data...");
  }
}

void loop() {
  // Handle button presses
  handleButtonPresses();
}

void handleButtonPresses() {
  // Button 1 (Left) - Rotate background colors
  if (digitalRead(WIO_KEY_A) == LOW) {
    delay(200);  // Debounce delay
    bgColorIndex = (bgColorIndex + 1) % (sizeof(colorOptions) / sizeof(colorOptions[0]));
    backgroundColor = colorOptions[bgColorIndex];
    tft.fillScreen(backgroundColor);
    // Redraw labels
    memset(labelPrinted, 0, sizeof(labelPrinted));
    drawInitialMetrics();
    delay(200);  // Delay to prevent rapid cycling
  }

  // Button 2 (Middle) - Rotate text colors
  if (digitalRead(WIO_KEY_B) == LOW) {
    delay(200);  // Debounce delay
    textColorIndex = (textColorIndex + 1) % (sizeof(colorOptions) / sizeof(colorOptions[0]));
    // Ensure text color is not the same as background color
    if (textColorIndex == bgColorIndex) {
      textColorIndex = (textColorIndex + 1) % (sizeof(colorOptions) / sizeof(colorOptions[0]));
    }
    textColor = colorOptions[textColorIndex];
    // Redraw labels and values
    memset(labelPrinted, 0, sizeof(labelPrinted));
    for (int i = 0; i < 8; i++) {
      prevMetricValues[i] = "";  // Force redraw of values
    }
    drawInitialMetrics();
    delay(200);  // Delay to prevent rapid cycling
  }

  // Button 3 (Right) - Save settings to SD card
  if (digitalRead(WIO_KEY_C) == LOW) {
    delay(200);  // Debounce delay
    saveColorSettings();
    if (DEBUG) {
      Serial.println("Color settings saved to SD card.");
    }
    // Provide visual feedback
    tft.fillRect(0, 0, tft.width(), tft.height(), backgroundColor);
    tft.setTextColor(textColor, backgroundColor);
    tft.setCursor(10, tft.height() / 2 - 10);
    tft.print("Settings Saved");
    delay(1000);
    // Redraw metrics
    memset(labelPrinted, 0, sizeof(labelPrinted));
    drawInitialMetrics();
  }
}

// Function to load color settings from SD card
void loadColorSettings() {
  File file = SD.open("/colors.txt", FILE_READ);
  if (file) {
    String bgIndexStr = file.readStringUntil('\n');
    String textIndexStr = file.readStringUntil('\n');

    bgColorIndex = bgIndexStr.toInt();
    textColorIndex = textIndexStr.toInt();

    // Ensure indices are within bounds
    if (bgColorIndex < 0 || bgColorIndex >= (sizeof(colorOptions) / sizeof(colorOptions[0]))) {
      bgColorIndex = 4;  // Default to blue background
    }
    if (textColorIndex < 0 || textColorIndex >= (sizeof(colorOptions) / sizeof(colorOptions[0]))) {
      textColorIndex = 5;  // Default to yellow text
    }
    // Ensure text color is not the same as background color
    if (textColorIndex == bgColorIndex) {
      textColorIndex = (textColorIndex + 1) % (sizeof(colorOptions) / sizeof(colorOptions[0]));
    }
    file.close();
    if (DEBUG) {
      Serial.println("Color settings loaded from SD card.");
    }
  } else {
    if (DEBUG) {
      Serial.println("No color settings file found. Using defaults.");
    }
  }
}

// Function to save color settings to SD card
void saveColorSettings() {
  File file = SD.open("/colors.txt", FILE_WRITE);
  if (file) {
    file.println(bgColorIndex);
    file.println(textColorIndex);
    file.close();
  } else {
    if (DEBUG) {
      Serial.println("Failed to open file for writing.");
    }
  }
}

// Function to draw initial placeholders on the TFT display
void drawInitialMetrics() {
  updateMetricDisplay("RAM_TOTAL", "---");
  updateMetricDisplay("RAM_FREE", "---");
  updateMetricDisplay("CPU_USAGE", "---");
  updateMetricDisplay("CPU_TEMP", "---");
  updateMetricDisplay("STORAGE_FR", "---");
  updateMetricDisplay("IP_ADDR", "---");
  updateMetricDisplay("USER_NAME", "sysop");  // Force user name to 'sysop'
  updateMetricDisplay("COMPUTER_N", "---");
}

// Function to receive data via I2C
void receiveData(int bytes) {
  if (DEBUG) {
    Serial.print("receiveData called with bytes=");
    Serial.println(bytes);
  }
  int initialBufferIndex = bufferIndex;

  while (Wire.available() && bufferIndex < MAX_BUFFER_SIZE) {
    char c = Wire.read();
    buffer[bufferIndex++] = c;
    if (DEBUG) {
      Serial.print("Read char: '");
      Serial.print(c);
      Serial.println("'");
    }
  }

  if (DEBUG) {
    Serial.print("bufferIndex after read: ");
    Serial.println(bufferIndex);
  }

  // Process buffer if bufferIndex >= MAX_BUFFER_SIZE
  if (bufferIndex >= MAX_BUFFER_SIZE) {
    buffer[MAX_BUFFER_SIZE] = '\0';  // Null-terminate
    if (DEBUG) {
      Serial.print("Raw Metric Data: '");
      Serial.print(buffer);
      Serial.println("'");
    }
    processMetric(buffer);  // Process the received metric
    bufferIndex = 0;  // Reset buffer index for the next metric
  }
}

// Function to trim trailing spaces from a string
void trimTrailingSpaces(char* str) {
  int len = strlen(str);
  while (len > 0 && isspace(str[len - 1])) {
    str[--len] = '\0';
  }
}

// Function to process a single metric and update the display
void processMetric(char* metric) {
  char metricName[11];   // 10 chars + null terminator
  char metricValue[16];  // 15 chars + null terminator

  // Ensure the metric buffer is the correct length
  if (strlen(metric) != MAX_BUFFER_SIZE) {
    if (DEBUG) {
      Serial.println("Invalid metric length received.");
    }
    return;
  }

  // Verify that the separator is a colon at position 10
  if (metric[10] != ':') {
    if (DEBUG) {
      Serial.println("Invalid metric format: missing separator.");
    }
    return;
  }

  // Extract metric name and value
  strncpy(metricName, metric, 10);
  metricName[10] = '\0';

  strncpy(metricValue, metric + 11, 15);
  metricValue[15] = '\0';

  // Trim trailing spaces from metricName and metricValue
  trimTrailingSpaces(metricName);
  trimTrailingSpaces(metricValue);

  if (DEBUG) {
    Serial.print("Metric Name: '");
    Serial.print(metricName);
    Serial.println("'");
    Serial.print("Metric Value: '");
    Serial.print(metricValue);
    Serial.println("'");
  }

  // Update the display
  updateMetricDisplay(metricName, metricValue);
}

// Function to update the display for a specific metric
void updateMetricDisplay(const char* metricName, const char* metricValue) {
  int yOffset = -1;
  const char* label = "";
  const char* unit = "";
  int metricIndex = -1;  // Index for prevMetricValues array

  if (strcmp(metricName, "RAM_TOTAL") == 0) {
    yOffset = 10;
    label = "RAM Total: ";
    unit = " MB";
    metricIndex = 0;
  }
  else if (strcmp(metricName, "RAM_FREE") == 0) {
    yOffset = 40; // Adjusted for larger font
    label = "RAM Free: ";
    unit = " MB";
    metricIndex = 1;
  }
  else if (strcmp(metricName, "CPU_USAGE") == 0) {
    yOffset = 70; // Adjusted for larger font
    label = "CPU Usage: ";
    unit = "%";
    metricIndex = 2;
  }
  else if (strcmp(metricName, "CPU_TEMP") == 0) {
    yOffset = 100;
    label = "CPU Temp: ";
    unit = " C";  // Replaced "°C" with " C" to avoid unsupported character
    metricIndex = 3;
  }
  else if (strcmp(metricName, "STORAGE_FR") == 0) {
    yOffset = 130;
    label = "Storage Free: ";
    unit = " GB";
    metricIndex = 4;
  }
  else if (strcmp(metricName, "IP_ADDR") == 0) {
    yOffset = 160;
    label = "IP Addr: ";
    unit = "";
    metricIndex = 5;
  }
  else if (strcmp(metricName, "USER_NAME") == 0) {
    yOffset = 190;
    label = "User: ";
    unit = "";
    metricIndex = 6;
    // Force metricValue to 'sysop'
    metricValue = "sysop";
  }
  else if (strcmp(metricName, "COMPUTER_N") == 0) {
    yOffset = 220;
    label = "Computer: ";
    unit = "";
    metricIndex = 7;
  }
  else {
    if (DEBUG) {
      Serial.print("Unknown metric name: '");
      Serial.print(metricName);
      Serial.println("'");
    }
    return;
  }

  // Combine metric value with unit
  String valueWithUnit = String(metricValue) + unit;

  // Check if the value has changed
  if (prevMetricValues[metricIndex] == valueWithUnit && labelPrinted[metricIndex]) {
    // Value hasn't changed; no need to update display
    return;
  }

  // Overwrite the old value with background color
  int valueXPosition = 10 + tft.textWidth(label);
  int valueWidth = tft.textWidth(prevMetricValues[metricIndex] + unit);

  tft.fillRect(valueXPosition, yOffset, valueWidth, 30, backgroundColor);

  // Update stored value
  prevMetricValues[metricIndex] = valueWithUnit;

  // Print label if not printed yet
  if (!labelPrinted[metricIndex]) {
    tft.setCursor(10, yOffset);
    tft.setTextColor(textColor, backgroundColor);
    tft.print(label);
    labelPrinted[metricIndex] = true;
  }

  // Print new value
  tft.setTextColor(textColor, backgroundColor);
  tft.setCursor(valueXPosition, yOffset);
  tft.print(valueWithUnit);

  if (DEBUG) {
    Serial.print("Updated metric on LCD: ");
    Serial.print(metricName);
    Serial.print(" -> ");
    Serial.println(valueWithUnit);
  }
}
