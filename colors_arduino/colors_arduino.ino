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

// Pins for inputs (push buttons) and outputs (shift registers / LEDs)
const int UP_PIN = A0;
const int RIGHT_PIN = 0; // TODO
const int DOWN_PIN = 0; // TODO
const int LEFT_PIN = 0; // TODO
const int FOO = 13; // TODO

const int colorMap[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // TODO
const int pinMap[4]]4]; // TODO

/* Initializes all connections and creates a new game. */
void setup() {
    // Initialize serial
    Serial.begin(9600);

    // Initialize inputs and outputs
    pinMode(UP_PIN, INPUT);
    pinMode(RIGHT_PIN, INPUT);
    pinMode(DOWN_PIN, INPUT);
    pinMode(LEFT_PIN, INPUT);
    pinMode(FOO, OUTPUT); // TODO

    // TODO ~ Seed the PRNG with something reasonable

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
    int upVal = digitalRead(UP_PIN);
    int rightVal = digitalRead(RIGHT_PIN);
    int downVal = digitalRead(DOWN_PIN);
    int leftVal = digitalRead(LEFT_PIN);

    // Confirm that there was no previous button press (so we don't respond too many times)
    if (!buttonPressed) {
        // Make a move
        if (upVal == HIGH) {
            buttonPressed = true;
            makeMove(UP);
        }
        else if (rightVal == HIGH) {
            buttonPressed = true;
            makeMove(RIGHT);
        }
        else if (downVal == HIGH) {
            buttonPressed = true;
            makeMove(DOWN);
        }
        else if (leftVal == HIGH) {
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
            // TODO
        break;

        case RIGHT:
            // TODO
        break;

        case DOWN:
            // TODO
        break;

        case LEFT:
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
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

    // Otherwise, it was valid, so add a new tile
    // TODO

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
    lostGame();
}

void lostGame() {
    // Update game state
    gameInProgress = false;

    // TODO ~ Do something to show they lost (womp womp)
}

void wonGame() {
    // Update game state
    gameInProgress = false;

    // TODO ~ Do something to show they won (woohooo!)
}

/****************************************
 ********** DISPLAY  METHODS ************
 ****************************************/

 void renderBoard() {
    // TODO ~ Hahahahahahahaha
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

 /* Randomly returns either 0 or 1. */
int randomTile() {
    return random(0, 2);

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
        return true;
    }

    return false;
}