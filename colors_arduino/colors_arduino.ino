/* colors_arduino.ino
 * by Angela Li
 *
 * Arduino controller for Colors, the best game ever! <3
 *
 * I/O and game logic. Listens for inputs (push buttons), updates game state, and sends
 * outputs (LEDs, via shift registers). Delegates sundry cuteness to a Node.js app
 * via serial.
 *
 */

const int UP = 0;
const int RIGHT = 1;
const int DOWN = 2;
const int LEFT = 3;

// Game state
int board[4][4];
int tempBoard[4][4];
bool gameInProgress = false;
bool gameEndingShown = false;
bool gameResult = false;
bool buttonPressed = false;

// Pins for inputs (push buttons)
const int UP_PIN = A0;
const int RIGHT_PIN = A1;
const int DOWN_PIN = A2;
const int LEFT_PIN = A3;
const int START_PIN = A4;

// Pins for outputs (shift registers / LEDs)
// Latch is brown, data is purple, clock is yellow
const int LATCH_PIN_ONE = 10;
const int DATA_PIN_ONE = 11;
const int CLOCK_PIN_ONE = 12;

const int LATCH_PIN_TWO = 4;
const int DATA_PIN_TWO = 5;
const int CLOCK_PIN_TWO = 6;

/* Initializes all connections and creates a new game. */
void setup() {
    // Initialize serial
    Serial.begin(9600);

    // Initialize inputs (push buttons) and outputs (shift registers and LEDs)
    pinMode(UP_PIN, INPUT);
    pinMode(RIGHT_PIN, INPUT);
    pinMode(DOWN_PIN, INPUT);
    pinMode(LEFT_PIN, INPUT);

    pinMode(LATCH_PIN_ONE, OUTPUT);
    pinMode(CLOCK_PIN_ONE, OUTPUT);
    pinMode(DATA_PIN_ONE, OUTPUT);

    pinMode(LATCH_PIN_TWO, OUTPUT);
    pinMode(CLOCK_PIN_TWO, OUTPUT);
    pinMode(DATA_PIN_TWO, OUTPUT);

    // Seed the PRNG with the reading from an unconnected pin
    randomSeed(analogRead(A4));

    // Begin game
    newGame();
}

/* Checks for user input. */
void loop() {
    // Make sure we have a game going on
    if (!gameInProgress && !gameEndingShown) {
        gameResult ? wonGame() : lostGame();
        return;
    }

    // Read inputs
    int upVal = digitalRead(UP_PIN);
    int rightVal = digitalRead(RIGHT_PIN);
    int downVal = digitalRead(DOWN_PIN);
    int leftVal = digitalRead(LEFT_PIN);
    int startVal = digitalRead(START_PIN);

    // Confirm that there was no previous button press (so we don't respond too many times)
    if (!buttonPressed) {
        // Currently playing a game -- make a move (maybe)
        if (gameInProgress) {
            if (upVal) {
                Serial.println(0);
                buttonPressed = true;
                makeMove(UP);
            }
            else if (rightVal) {
                Serial.println(1);
                buttonPressed = true;
                makeMove(RIGHT);
            }
            else if (downVal) {
                Serial.println(2);
                buttonPressed = true;
                makeMove(DOWN);
            }
            else if (leftVal) {
                Serial.println(3);
                buttonPressed = true;
                makeMove(LEFT);
            }
            else if (startVal) {
                Serial.println(4);
                buttonPressed = true;
                newGame();
            }
        }

        // Not currently playing a game -- start a new one (maybe)
        else if (startVal) {
            Serial.println(4);
            buttonPressed = true;
            newGame();
        }
    }

    // Reset button state if necessary
    else if (!upVal && !rightVal && !downVal && !leftVal && !startVal) {
        buttonPressed = false;
    }

    delay(100);
}

/****************************************
 ********** GAME METHODS ****************
 ****************************************/

/* Resets internal game state and display. */
void newGame() {
    // Clear board
    memset(board, 0, sizeof(board));

    // Add |initialTiles| new tiles to the board
    int initialTiles = 2;
    for (int i = 0; i < initialTiles; i++) {
        int tile = randomTile();
        int locX = randomIndex();
        int locY = randomIndex();

        board[locX][locY] = tile;
    }

    // Update game state
    gameInProgress = true;

    // Update display
    renderBoard();
}

/* Updates internal game state and display. */
void makeMove(int dir) {
    boolean validMove = attemptMove(dir, true);

    // Don't do anything if the move wasn't valid
    if (!validMove) {
        return;
    }

    // Grab the updated board from |tempBoard|
    memcpy(board, tempBoard, sizeof(board));

    // Update display
    renderBoard();

    // Check if there are any valid moves left
    checkLost();
}

/* Returns whether or not a given move is valid on the current board.
 * Stores the updated board (with a new inserted tile) in |tempBoard|. */
boolean attemptMove(int dir, bool madeByUser) {
    // Clone our current board so we can work with it
    memcpy(tempBoard, board, sizeof(board));

    // Keep track of whether anything moves
    boolean movedAnythingYet = false;

    switch(dir) {
        case UP:
            for (int j = 0; j <= 3; j++) {
                for (int i = 0; i <= 3; i++) {
                    if (i == 0) {
                        continue;
                    }
                    movedAnythingYet = attemptTileMove(i, j, i - 1, j, madeByUser) || movedAnythingYet;
                }
            }
        break;

        case RIGHT:
            for (int i = 0; i <= 3; i++) {
                for (int j = 3; j >= 0; j--) {
                    if (j == 3) {
                        continue;
                    }
                    movedAnythingYet = attemptTileMove(i, j, i, j + 1, madeByUser) || movedAnythingYet;
                }
            }
        break;

        case DOWN:
            for (int j = 0; j <= 3; j++) {
                for (int i = 3; i >= 0; i--) {
                    if (i == 3) {
                        continue;
                    }
                    movedAnythingYet = attemptTileMove(i, j, i + 1, j, madeByUser) || movedAnythingYet;
                }
            }
        break;

        case LEFT:
            for (int i = 0; i <= 3; i++) {
                for (int j = 0; j <= 3; j++) {
                    if (j == 0) {
                        continue;
                    }
                    movedAnythingYet = attemptTileMove(i, j, i, j - 1, madeByUser) || movedAnythingYet;
                }
            }
        break;
    }

    // If no tiles moved, this was an invalid move
    if (!movedAnythingYet) {
        return false;
    }

    // Add a new tile
    insertTile();

    return true;
}

/* Returns whether tile at (i, j) can be moved to (i2, j2).
 * Updates |tempBoard| if a move is possible. */
bool attemptTileMove(int i, int j, int i2, int j2, bool madeByUser) {
    // No tile in the original space
    if (tempBoard[i][j] == 0) {
        return false;
    }

    // No tile in the new space
    if (tempBoard[i2][j2] == 0) {
        tempBoard[i2][j2] = tempBoard[i][j];
        tempBoard[i][j] = 0;
        return true;
    }

    // Identical tiles, collapse them into the new location
    if (tempBoard[i][j] == tempBoard[i2][j2]) {
        tempBoard[i][j] = 0;
        tempBoard[i2][j2]++;

        // They won!
        if (tempBoard[i2][j2] == 7 && madeByUser) {
            gameResult = true;
            gameInProgress = false;
            gameEndingShown = false;
        }

        return true;
    }

    return false;
}

/* Inserts a tile into |tempBoard|. */
void insertTile() {
    // Keep track of empty locations
    int numEmpty = 0;
    int locations[16];
    memset(locations, 0, sizeof(locations));

    // Find empty locations
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
            if (tempBoard[i][j] == 0) {
                locations[numEmpty] = (i * 4) + j;
                numEmpty++;
            }
        }
    }

    // Choose a random empty location to insert a tile
    int location = locations[random(0, numEmpty)];
    int i = location / 4;
    int j = location % 4;
    tempBoard[i][j] = randomTile();

    return;
}

/* Checks whether the game is still playable. */
void checkLost() {
    for (int i = 0; i < 4; i++) {
        if (attemptMove(i, false)) {
            return;
        }
    }

    // Whoops, we lost the game
    gameInProgress = false;
    gameResult = false;
    gameEndingShown = false;
}

/****************************************
 ********** DISPLAY  METHODS ************
 ****************************************/

// (off, red, red-green, green, green-blue, blue, red-blue, white)
byte colors[8] = {7, 6, 4, 5, 1, 3, 2, 0};

byte color(int tileNum) {
    tileNum -= 1;
    int i = tileNum / 4;
    int j = tileNum % 4;
    int value = board[i][j];
    return colors[value];
}

void renderBoard() {
    byte A =  color( 1) * 2 +
              color( 2) * 16 +
            ((color( 3) % 2) * 128) +
              color(15) % 2;
    byte B = (color( 3) / 2) * 2 +
              color( 4) * 8 +
             (color( 5) % 4) * 64 +
             (color(15) / 2) % 2;
    byte C = (color( 5) / 4) * 2 +
              color( 6) * 4 +
              color( 7) * 32 +
              color(15) / 4;

    byte D =  color( 8) * 2 +
              color( 9) * 16 +
             (color(10) % 2) * 128 +
              color(16) % 2;
    byte E = (color(10) / 2) * 2 +
              color(11) * 8 +
             (color(12) % 4) * 64 +
             (color(16) / 2 % 2);
    byte F = (color(12) / 4) * 2 +
              color(13) * 4 +
              color(14) * 32 +
              color(16) / 4;

    digitalWrite(LATCH_PIN_ONE, LOW);
    shiftOut(DATA_PIN_ONE, CLOCK_PIN_ONE, MSBFIRST, C);
    shiftOut(DATA_PIN_ONE, CLOCK_PIN_ONE, MSBFIRST, B);
    shiftOut(DATA_PIN_ONE, CLOCK_PIN_ONE, MSBFIRST, A);
    digitalWrite(LATCH_PIN_ONE, HIGH);

    digitalWrite(LATCH_PIN_TWO, LOW);
    shiftOut(DATA_PIN_TWO, CLOCK_PIN_TWO, MSBFIRST, F);
    shiftOut(DATA_PIN_TWO, CLOCK_PIN_TWO, MSBFIRST, E);
    shiftOut(DATA_PIN_TWO, CLOCK_PIN_TWO, MSBFIRST, D);
    digitalWrite(LATCH_PIN_TWO, HIGH);
}

/* Prints board to serial. For debugging. */
void printBoard() {
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
            Serial.print(board[i][j]);
        }
        Serial.println("");
    }
 }

/* Show player that they won (woohoo!), and restart the game. */
void wonGame() {
    Serial.println("W");

    // Fill board with white
    fillBoard(7);

    // Much excite
    gameEndingShown = true;
    return;
}

/* Show player that they lost (womp womp), and restart the game. */
void lostGame() {
    Serial.println("L");

    // Fill board with red
    fillBoard(1);

    // Very wow
    gameEndingShown = true;
    return;
}

void allOn() {
    memset(board, 7, sizeof(board));
    renderBoard();
}

void allOff() {
    memset(board, 0, sizeof(board));
    renderBoard();
}

void fillBoard(int color) {
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
            board[i][j] = color;
            renderBoard();
            delay(500);
        }
    }
}

/****************************************
********** UTILITY METHODS *************
****************************************/

/* Randomly returns either 1 or 2. */
int randomTile() {
    return random(1, 3);
}

/* Randomly returns the index of a row or column. */
int randomIndex() {
    return random(0, 4);
}