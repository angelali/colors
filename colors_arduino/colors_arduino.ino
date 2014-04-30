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
boolean gameResult = false;
boolean buttonPressed = false;

// Pins for inputs (push buttons)
const int UP_PIN = A0;
const int RIGHT_PIN = A1;
const int DOWN_PIN = A2;
const int LEFT_PIN = A3;

// Pins for outputs (shift registers / LEDs)
const int LATCH_PIN = 8; // Green wire
const int CLOCK_PIN = 12; // Yellow wire
const int DATA_PIN = 11; // Blue wire

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

    // Seed the PRNG with the reading from an unconnected pin
    randomSeed(analogRead(A4));

    // Begin game
    newGame();
}

/* Checks for user input. */
void loop() {
    /* BEGIN DEBUGGING */
    for (int i = 0; i < 8; i++) {
      board[0][0] = i;
      board[0][1] = i;
      board[0][2] = i;
      board[0][3] = i;
      renderBoard();
      delay(500);
    }
    return; 
    /* END DEBUGGING */
  
    // Make sure we have a game going on
    if (!gameInProgress) {
        gameResult ? wonGame() : lostGame();
        return;
    }

    // Read inputs
    int upVal = digitalRead(UP_PIN);
    int rightVal = digitalRead(RIGHT_PIN);
    int downVal = digitalRead(DOWN_PIN);
    int leftVal = digitalRead(LEFT_PIN);

    // Confirm that there was no previous button press (so we don't respond too many times)
    if (!buttonPressed) {
        // Make a move
        if (upVal) {
            Serial.println("UP");
            buttonPressed = true;            
            makeMove(UP);
        }
        else if (rightVal) {
            Serial.println("RIGHT");          
            buttonPressed = true;
            makeMove(RIGHT);
        }
        else if (downVal) {
            Serial.println("DOWN");          
            buttonPressed = true;
            makeMove(DOWN);
        }
        else if (leftVal) {
            Serial.println("LEFT");          
            buttonPressed = true;
            makeMove(LEFT);
        }
    }

    // Reset button state if necessary
    else if (!upVal && !rightVal && !downVal && !leftVal) {
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
    memcpy(board, blankBoard, sizeof(board));
    
    /* BEGIN DEBUGGING */
    renderBoard();
    return;
    /* END DEBUGGING */

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

    // Add a new tile
    insertTile();

    return true;
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
            gameResult = true;
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
    int insertLocation = random(0, numEmpty);
    int i = insertLocation / 4;
    int j = insertLocation % 4;
    tempBoard[i][j] = randomTile();

    return;
}

/* Checks whether the game is still playable. */
void checkLost() {
    for (int i = 0; i < 4; i++) {
        if (attemptMove(i)) {
            return;
        }
    }

    // Whoops, we lost the game
    gameInProgress = false;
    gameResult = false;
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
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
            Serial.print(board[i][j]);
        }
        Serial.println("");
    }
 }

 void wonGame() {
    Serial.println("W");

    // TODO ~ Show that they won (woohoo!)

    return;
 }

 void lostGame() {
    Serial.println("L");

    // TODO ~ Show that they lost (womp womp)

    return;
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
