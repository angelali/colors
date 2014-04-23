/* colors_arduino.ino
 * by Angela Li
 *
 * Arduino controller for Colors, the best game ever! <3 <3 <3 <3 <3
 *
 * I/O and game logic. Listens for inputs (push buttons), updates game state, and sends
 * outputs (LEDs, via shift registers). Delegates sundry cuteness to a Node.js app
 * via serial.
 *
 */

// Constants are fun!
const int UP = 0;
const int RIGHT = 1;
const int DOWN = 2;
const int LEFT = 3;

// Game state
int board[4][4];
boolean gameInProgress = false;
boolean buttonPressed = false;

// Pins for inputs (push buttons) and outputs (shift registers / LEDs)
const int UP_PIN = A0;
const int RIGHT_PIN = 0; // TODO
const int DOWN_PIN = 0; // TODO
const int LEFT_PIN = 0; // TODO
const int FOO = 13; // TODO

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

/* Randomly returns either 0 or 1. */
int randomTile() {
    return random(0, 2); /* OMG WOW CHECK THIS ONE OUT */

}

/* Randomly returns the index of a row or column. */
int randomIndex() {
    return random(0, 4);
}

/* Resets internal game state and display. */
void newGame() {
    // Clear board
    board = [[0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]];

    // Add |initialTiles| new tiles to the board
    int initialTiles = 2;
    for (int i = 0; i < initialTiles; i++) {
        int tile = randomTile();
        int locX = randomIndex();
        int locY = randomIndex();

        board[locX][locY] = tile;
    }

    // Update game state
    gameinProgress = true;

    // Update display
    renderBoard();
}

/* Updates internal game state and display. */
void makeMove(int dir) {
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
            // TODO
        break;
    }

    // Update display
    renderBoard();
}

void endGame() {
    // Update game state
    gameInProgress = false;
}

/****************************************
 ********** DISPLAY  METHODS ************
 ****************************************/

 void renderBoard() {
    // DEBUGGING ~ Print board to serial
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            Serial.print(board[i][j]);
        }
        Serial.println("");
    }

    // (GIANT FUCKING) TODO ~ Display the board
 }