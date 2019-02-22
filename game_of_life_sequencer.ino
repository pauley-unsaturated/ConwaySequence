#include <Adafruit_Keypad.h>
#include <Adafruit_NeoPixel.h>

const byte ROWS = 4; // four rows
const byte COLS = 8; // eight columns
//define the symbols on the buttons of the keypads

byte trellisKeys[ROWS][COLS] = {
  {0,  1,  2,  3,  4,  5,  6,  7},
  {8,  9,  10, 11, 12, 13, 14, 15},
  {16, 17, 18, 19, 20, 21, 22, 23},
  {24, 25, 26, 27, 28, 29, 30, 31}
};

int key_row(int key) {
  return key / 8;
}

int key_col(int key) {
  return key % 8;
}

byte rowPins[ROWS] = {14, 15, 16, 17}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4, 5, 6, 7, 8, 9}; //connect to the column pinouts of the keypad

#define NEOPIXEL_PIN 10

//initialize an instance of class NewKeypad
Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(trellisKeys), rowPins, colPins, ROWS, COLS);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(32, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

byte conway_grid[2][ROWS][COLS];
int cur_grid_idx = 0;

const int tick_len_ms = 10;
const int ticks_per_step = 20;



int ticks = 0;

void setup() {
  Serial.begin(115200);
  customKeypad.begin();
  strip.begin();
  strip.show();
}

void loop() {
  customKeypad.tick();
  bool updated = false;
  while (customKeypad.available()) {
    keypadEvent e = customKeypad.read();
    int key = (int)e.bit.KEY;
    Serial.print(key);
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      Serial.println(" pressed");
      if (get_grid_cell(key_row(key), key_col(key))) {
        unset_grid_cell(key_row(key), key_col(key));
      }
      else {
        set_grid_cell(key_row(key), key_col(key));
      }
    }
    else if (e.bit.EVENT == KEY_JUST_RELEASED) {
      Serial.println(" released");
    }

    updated = true;
  }
  
  ticks++;
  if (ticks == ticks_per_step) {
    ticks = 0;
    do_step();
    updated = true;
  }

  if (updated) {
    update_display();
  }

  delay(tick_len_ms);
}

byte get_grid_cell(int key_row, int key_col) {
  return conway_grid[cur_grid_idx][key_row][key_col];
}

void set_grid_cell(int row, int col) {
  conway_grid[cur_grid_idx][row][col] = 1;
}

void unset_grid_cell(int row, int col) {
  conway_grid[cur_grid_idx][row][col] = 0;
}

int neighbors_grid_cell(int row, int col) {
  int min_row = max(row - 1, 0);
  int max_row = min(row + 1, ROWS - 1);
  int min_col = max(col - 1, 0);
  int max_col = min(col + 1, COLS - 1);
  int sum = 0;
  
  for (int i = min_row; i <= max_row; i++) {
    for (int j = min_col; j <= max_col; j++) {
      sum += get_grid_cell(i, j);
    }
  }
  if (get_grid_cell(row, col)) sum--;

  return sum;
}

void do_step() {
  int new_idx = (cur_grid_idx + 1) % 2;

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      byte value = 0;
      int neighbors = neighbors_grid_cell(i, j);
      if (get_grid_cell(i, j)) {
        if ( neighbors > 1 && neighbors < 4) {
          value = 1;
        }
      }
      else {
        if ( neighbors == 3 ) {
          value = 1;
        }
      }
      conway_grid[new_idx][i][j] = value;
    }
  }
  
  cur_grid_idx = new_idx;
}

void update_display() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      if (conway_grid[cur_grid_idx][i][j] > 0) {
        strip.setPixelColor(i * COLS + j, 0, 255, 0);
      }
      else {
        strip.setPixelColor(i * COLS + j, 0, 0, 0);
      }
    }
  }
  strip.show();
}
