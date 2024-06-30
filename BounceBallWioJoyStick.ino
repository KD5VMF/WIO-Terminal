#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();  // Create display object

const int radius = 10;  // Ball radius
int x = 160;  // Initial x position of the ball
int y = 120;  // Initial y position of the ball
int prevX = x;  // Previous x position of the ball
int prevY = y;  // Previous y position of the ball

void setup() {
  // Initialize display
  tft.init();
  tft.setRotation(3);  // Adjust rotation if necessary
  tft.fillScreen(TFT_BLACK);

  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Initialize button pins
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
}

void loop() {
  // Read button states
  bool up = !digitalRead(WIO_5S_UP);
  bool down = !digitalRead(WIO_5S_DOWN);
  bool left = !digitalRead(WIO_5S_LEFT);
  bool right = !digitalRead(WIO_5S_RIGHT);

  // Debugging output
  Serial.print("Up: ");
  Serial.print(up);
  Serial.print(" Down: ");
  Serial.print(down);
  Serial.print(" Left: ");
  Serial.print(left);
  Serial.print(" Right: ");
  Serial.println(right);

  // Update ball position based on button states
  if (up) {
    y -= 5;
  }
  if (down) {
    y += 5;
  }
  if (left) {
    x -= 5;
  }
  if (right) {
    x += 5;
  }

  // Constrain ball position to screen boundaries
  x = constrain(x, radius, tft.width() - radius);
  y = constrain(y, radius, tft.height() - radius);

  // Erase the previous ball position
  tft.fillCircle(prevX, prevY, radius, TFT_BLACK);

  // Draw the ball at the new position
  tft.fillCircle(x, y, radius, TFT_WHITE);

  // Update previous ball position
  prevX = x;
  prevY = y;

  // Short delay for smoother movement
  delay(50);
}
