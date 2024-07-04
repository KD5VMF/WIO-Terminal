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

#include "TFT_eSPI.h" // Graphics and display library
#include <WiFi.h>     // Wi-Fi library

TFT_eSPI tft = TFT_eSPI();  // Create a display object

const int N = 8; // Dimension of the matrix
int matrixA[N][N], matrixB[N][N], matrixResult[N][N];

void setup() {
  tft.begin();
  tft.setRotation(3); // Set the display orientation
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  WiFi.begin(); // Start Wi-Fi
}

void loop() {
  scanNetworksAndSeedRandom(); // Scan Wi-Fi networks to seed the random number generator
  fillMatricesWithRandom();    // Fill matrices with random numbers
  multiplyMatrices();          // Multiply matrices
  displayMatrices();           // Display matrices on the screen
  delay(1000);                 // Wait 5 seconds before repeating
}

// Function to scan Wi-Fi networks and seed the random number generator
void scanNetworksAndSeedRandom() {
  int n = WiFi.scanNetworks();
  long seed = n;
  for (int i = 0; i < n; ++i) {
    seed += WiFi.RSSI(i); // Use RSSI values to adjust the seed
  }
  randomSeed(seed);
}

// Function to fill matrices with random numbers
void fillMatricesWithRandom() {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      matrixA[i][j] = random(0, 100); // Fill with random numbers between 0 and 99
      matrixB[i][j] = random(0, 100);
    }
  }
}

// Function to multiply matrices
void multiplyMatrices() {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      matrixResult[i][j] = 0;
      for (int k = 0; k < N; k++) {
        matrixResult[i][j] += matrixA[i][k] * matrixB[k][j];
      }
    }
  }
}

// Function to display matrices on the TFT screen
void displayMatrices() {
  tft.fillScreen(TFT_BLACK); // Clear the screen
  
  // Starting positions for matrices
  int startX_A = 50, startY_A = 30; // Top left for Matrix A
  int startX_B = 180, startY_B = 30; // Top right for Matrix B, assuming B next to A
  int startX_Result = 105, startY_Result = 140; // Below A and B, centered if possible
  
  displayMatrix(matrixA, startX_A, startY_A); // Display Matrix A
  displayMatrix(matrixB, startX_B, startY_B); // Display Matrix B
  displayMatrix(matrixResult, startX_Result, startY_Result); // Display Result matrix
}

// Function to display a matrix on the TFT screen
void displayMatrix(int matrix[N][N], int startX, int startY) {
  int charWidth = 6; // Width of each character in pixels at textSize 1
  int charHeight = 10; // Height of each character in pixels at textSize 1
  String text; // To hold the text for each number
  
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      text = String(matrix[i][j]);
      tft.drawString(text, startX + j * (charWidth * 2), startY + i * charHeight, 1);
    }
  }
}
