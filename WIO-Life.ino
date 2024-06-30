#include "TFT_eSPI.h" // Graphics and font library for ST7735 driver chip

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#define GRID_WIDTH  64
#define GRID_HEIGHT  44  // Adjusted for text display area below the iteration count
#define CELL_SIZE    5
#define TEXT_HEIGHT  16  // Height reserved for text display at the top

bool currentGrid[GRID_WIDTH][GRID_HEIGHT];
bool nextGrid[GRID_WIDTH][GRID_HEIGHT];
int unchangedIterations = 0; // Counter for unchanged iterations
int iterationCount = 0; // Count the number of iterations
int maxIterations = 500; // Maximum iterations before automatic reset

void setup() {
  randomSeed(analogRead(0)); // Initialize random number generator
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2); // Ensure text size is set for the iteration count display
  initializeGrid(true); // Randomly initialize the grid with a true full restart
}

void loop() {
  bool hasChanged = false;
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      int aliveNeighbors = countAliveNeighbors(x, y);

      bool nextState = currentGrid[x][y];
      if (currentGrid[x][y] == true && (aliveNeighbors < 2 || aliveNeighbors > 3)) {
        nextState = false;
      } else if (currentGrid[x][y] == false && aliveNeighbors == 3) {
        nextState = true;
      }

      if (nextState != currentGrid[x][y]) {
        hasChanged = true;
      }
      nextGrid[x][y] = nextState;
    }
  }

  // If no change for 5 iterations or if maxIterations reached, reinitialize
  if (!hasChanged) {
    unchangedIterations++;
  } else {
    unchangedIterations = 0; // Reset counter if there's any change
  }
  iterationCount++; // Always increment iteration count

  if (unchangedIterations >= 5 || iterationCount >= maxIterations) {
    initializeGrid(false); // Reinitialize grid for a new game without clearing screen
    iterationCount = 0; // Reset iteration count
    unchangedIterations = 0; // Reset unchanged iterations count
  } else {
    updateDisplay(); // Update display only if not resetting
  }

  delay(100); // Slow down the simulation
}

void initializeGrid(bool fullRestart) {
  if (fullRestart) {
    tft.fillScreen(TFT_BLACK); // Clear the screen fully only on full restart
  }
  
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      currentGrid[x][y] = random(2); // Randomly initialize grid cells
      nextGrid[x][y] = currentGrid[x][y]; // Ensure next grid matches initial state to avoid immediate clearing
    }
  }

  // Reset counters for a fresh start
  iterationCount = 0;
  unchangedIterations = 0;
  
  // Immediately update display to reflect the new grid
  updateDisplay();
}

void updateDisplay() {
  // Update the iteration count display
  tft.setCursor(0, 0);
  tft.fillRect(0, 0, tft.width(), TEXT_HEIGHT, TFT_BLACK); // Clear the text display area
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("Iterations: ");
  tft.print(iterationCount);

  // Redraw the entire grid
  for (int x = 0; x < GRID_WIDTH; x++) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
      currentGrid[x][y] = nextGrid[x][y]; // Update current grid state
      int aliveNeighbors = countAliveNeighbors(x, y);
      uint16_t color = getColorBasedOnNeighbors(currentGrid[x][y], aliveNeighbors);
      tft.fillRect(x * CELL_SIZE, (y * CELL_SIZE) + TEXT_HEIGHT, CELL_SIZE, CELL_SIZE, color);
    }
  }
}

int countAliveNeighbors(int x, int y) {
  int count = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (i == 0 && j == 0) continue; // Skip the cell itself
      int col = (x + i + GRID_WIDTH) % GRID_WIDTH;
      int row = (y + j + GRID_HEIGHT) % GRID_HEIGHT;
      count += currentGrid[col][row];
    }
  }
  return count;
}

uint16_t getColorBasedOnNeighbors(bool cellState, int aliveNeighbors) {
  if (!cellState) {
    return TFT_BLACK; // Dead cells are black
  }

  // Alive cells' colors based on the number of alive neighbors
  switch (aliveNeighbors) {
    case 0: return TFT_BLUE;
    case 1: return TFT_CYAN;
    case 2: return TFT_GREEN;
    case 3: return TFT_YELLOW;
    case 4: return TFT_ORANGE;
    case 5: return TFT_RED;
    case 6: return TFT_MAGENTA;
    case 7: return TFT_PURPLE;
    case 8: return TFT_WHITE;
    default: return TFT_WHITE; // This should never happen
  }
}
