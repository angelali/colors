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
const int blankBoard[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
int board[4][4];
int tempBoard[4][4];
boolean gameInProgress = false;
boolean buttonPressed = false;

// Pins for inputs (push buttons)
const int UP_PIN = A0;
const int RIGHT_PIN = A1;
const int DOWN_PIN = A2;
const int LEFT_PIN = A3;

// Pins for outputs (shift registers / LEDs)
const int LATCH_PIN = 8; // green
const int CLOCK_PIN = 12; // yellow
const int DATA_PIN = 11; // blue

/* Initializes all connections and creates a new game. */
void setup() {
    // Initialize serial
    Serial.begin(9600);

    // Initialize inputs (push buttons) and outputs (shift registers and LEDs)
    pinMode(UP_PIN, INPUT);
    pinMode(RIGHT_PIN, INPUT);
    pinMode(DOWN_PIN, INPUT);
    pinMode(LEFT_PIN, INPUT);

    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);

    // Seed the PRNG with the read from an unconnected pin
    randomSeed(analogRead(A4));

    // Begin game
    newGame();
}

/* Checks for user input. */
void loop() {
    // Make sure we have a game going on
    if (!gameInProgress) {
        return;
    }

    // Read inputs
    int upVal = analogRead(UP_PIN);
    int rightVal = analogRead(RIGHT_PIN);
    int downVal = analogRead(DOWN_PIN);
    int leftVal = analogRead(LEFT_PIN);

    // Confirm that there was no previous button press (so we don't respond too many times)
    if (!buttonPressed) {
        // Make a move
        if (upVal) {
            buttonPressed = true;
            makeMove(UP);
        }
        else if (rightVal) {
            buttonPressed = true;
            makeMove(RIGHT);
        }
        else if (downVal) {
            buttonPressed = true;
            makeMove(DOWN);
        }
        else if (leftVal) {
            buttonPressed = true;
            makeMove(LEFT);
        }
    }

    // Reset button state if necessary
    else if (!upVal && !rightVal && !downVal && !leftVal) {
        buttonPressed = false;
    }
}

/****************************************
 ********** GAME METHODS ****************
 ****************************************/

/* Resets internal game state and display. */
void newGame() {
    // Clear board
    memcpy(board, blankBoard, sizeof(board));

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
    boolean validMove = attemptMove(dir);

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
boolean attemptMove(int dir) {
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
                    movedAnythingYet = attemptTileMove(i, j, i - 1, j) || movedAnythingYet;
                }
            }
        break;

        case RIGHT:
            for (int i = 0; i <= 3; i++) {
                for (int j = 3; j >= 0; j--) {
                    if (j == 3) {
                        continue;
                    }
                    movedAnythingYet = attemptTileMove(i, j, i, j + 1) || movedAnythingYet;
                }
            }
        break;

        case DOWN:
            for (int j = 0; j <= 3; j++) {
                for (int i = 3; i >= 0; i--) {
                    if (i == 3) {
                        continue;
                    }
                    movedAnythingYet = attemptTileMove(i, j, i + 1, j) || movedAnythingYet;
                }
            }
        break;

        case LEFT:
            for (int i = 0; i <= 3; i++) {
                for (int j = 0; j <= 3; j++) {
                    if (j == 0) {
                        continue;
                    }
                    movedAnythingYet = attemptTileMove(i, j, i, j - 1) || movedAnythingYet;
                }
            }
        break;
    }

    // If no tiles moved, this was an invalid move
    if (!movedAnythingYet) {
        return false;
    }

    // TODO ~ Insert new tile

    return true;
}

/* Checks whether the game is still winnable. Calls lostGame() if not. */
void checkLost() {
    for (int i = 0; i < 4; i++) {
        if (attemptMove(i)) {
            return;
        }
    }

    // Whoops, we lost the game
    gameInProgress = false;
    Serial.println("L");
}

/****************************************
 ********** DISPLAY  METHODS ************
 ****************************************/

byte colors[8] = {7, 6, 4, 5, 1, 3, 2, 0};

byte color(int tileNum) {
    tileNum -= 1;
    int i = tileNum / 4;
    int j = tileNum % 4;
    int value = board[i][j];
    return colors[value];
}

void renderBoard() {
    byte A = color(15) % 2     << 0
           + color( 1)         << 1
           + color( 2)         << 4
           + color( 3) % 2     << 7;

    byte B = color(15) % 4 / 2 << 0
           + color( 3) / 2     << 1
           + color( 4)         << 3
           + color( 5) % 4     << 6;

    byte C = color(15) / 4     << 0
           + color( 5) / 4     << 1
           + color( 6)         << 2
           + color( 7)         << 5;

    byte D = color(16) % 2     << 0
           + color( 8)         << 1
           + color( 9)         << 4
           + color(10) % 2     << 7;

    byte E = color(16) % 4 / 2 << 0
           + color(10) / 2     << 1
           + color(11)         << 3
           + color(12) % 4     << 6;

    byte F = color(16) / 4     << 0
           + color(12) / 4     << 1
           + color(13)         << 2
           + color(14)         << 5;

    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, F);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, E);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, D);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, C);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, B);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, A);
    digitalWrite(LATCH_PIN, HIGH);
 }

/* Prints board to serial. For debugging. */
void printBoard() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            Serial.print(board[i][j]);
        }
        Serial.println("");
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

/* Returns whether tile at (i, j) can be moved to (i2, j2).
 * Updates |tempBoard| if a move is possible. */
boolean attemptTileMove(int i, int j, int i2, int j2) {
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
        if (tempBoard[i2][j2] == 6) {
            gameInProgress = false;
            Serial.println("W");
        }

        return true;
    }

    return false;
}