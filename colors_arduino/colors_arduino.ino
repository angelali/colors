/* colors_arduino.ino
 * by Angela Li
 *
 * Arduino controller for Colors, the best game ever! <3
 *
 * Game logic and I/O. Listens for inputs (push buttons), updates game state, and sends
 * outputs (LEDs, via shift registers). Delegates sundry cuteness to a Node.js app
 * via serial.
 *
 */

// Pins for inputs (push buttons)
const int UP = A0;
const int RIGHT = 0;
const int LEFT = 0;
const int DOWN = 0;

// Pins for outputs (shift registers / LEDs)
const int FOO = 13;

// Other useful things
boolean buttonPressed = false;

void setup() {
  // Initialize serial
  Serial.begin(9600);

  // Initialize inputs
  pinMode(UP, INPUT);

  // Initialize outputs
  pinMode(FOO, OUTPUT);
}

void loop() {
  int up = digitalRead(UP);
  int right = 0; // TODO
  int down = 0; // TODO
  int left = 0; // TODO

  // Make sure there was no previous button press (i.e. firing too many times)
  if (!buttonPressed) {
    if (up == HIGH) {
      buttonPressed = true;
      digitalWrite(FOO, HIGH);
    }

    else {
      digitalWrite(FOO, LOW);
    }

//    else if (0) {
//      // TODO
//    }
//    else if (0) {
//      // TODO
//    }
//    else if (0) {
//      // TODO
//    }
  }

  // Check whether button state can be reset
  else {
    if (!up && !right && !down && !left)
      buttonPressed = false;
  }
}
