/*
 * Arduino UNO Pin Switch Program.
 * Created by Jarod Brennfleck.
 * Do not steal credit bro. Effort was put into this.
 */

const int pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // All the pins we are going to use ina nice array
const int pinCount = 14; // The amount of pins we are using
const int captureDelay = 4; // The milliseconds we must wait before we capture again
const int capturePoints = 1000 / captureDelay; // Using the capture delay, determine how many capture points we need

int clearData[] = {-1, -1}; // Data for when we want to clear the screen
boolean clearing = false; // Determines whether we should clear the screen, ie, replace the clear data array's data
char readingIO = 'n'; // Determines whether we should read to change a pin's IO status
boolean verbose = true; // Whether we should be loud in telling the world about what we are receiving/doing
int pinState[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Contains the data of all the pin's states
int pinModes[] = {INPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT}; // Contains the data of a pins IO mode
boolean capturing = false; // Determines whether or not we are meant to be capturing data
int captured[pinCount][capturePoints]; // Generate an array of all the capture points we need
int captureIndex = 0; // Helps us know whichcapture point we are up to
int startCaptureTime = -1; // Determines what time we started capturing at

// Called whenever the Arduino is turned on, or the Arduino's reset button is pressed.
void setup() {
  for(int i = 0; i < pinCount; i++) { // Loop through every pin, making it an output, and turning it 'off'
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
  Serial.begin(9600); // Open the serial port.
  delay(1500); // Wait a sec...
  Serial.println("\n\nControl the arduino pins [0] to [13]. All pins are set to output."); // Now talk.
  Serial.println("Character to pin, and pin statuses are as follows:");
  printPins();
  printHelp();
}

// This method is called every time it finishes, hence the name 'loop'. It is used to detect input on any input moded pins.
void loop() {
  for(int pin = 0; pin < pinCount; pin++) {
    if(pinModes[pin] == INPUT) {
      pinState[pin] = digitalRead(pin); // Read the pin's data and set it if it's an input
    }
  }
  if(capturing) { // Are we meant to be capturing?
    if(startCaptureTime <= -1) startCaptureTime = millis(); // If it's the first capture, get our starting time.
    if(millis() - startCaptureTime >= captureIndex * captureDelay) { // If the elapsed time is greater than our current index * delay
      for(int i = 0; i < pinCount; i++) { // Grab this index's power
        captured[captureIndex][i] = pinState[i];
      }
      captureIndex++; // Increment our index
      if(captureIndex >= capturePoints) { // If we've finished, reset our values
        capturing = false; // We're not capturing anymore
        captureIndex = 0; // Our "next" index is 0
        startCaptureTime = -1; // Our "next" start time will need to be reset
      }
    }
  }
}

// This method is called if there is a serial event coming from you to the Arduino, like a pin state change.
void serialEvent() {
  while(Serial.available()) { // While the serial is available
    int inChar = Serial.read(); // Read the next character as it's ASCII keycode
    char realChar = (char)inChar; // Turn it into the ASCII character
    
    if(verbose) { // If we are loud, tell the user what key we got
      Serial.print("Received: ");
      Serial.print(inChar);
      Serial.print(", ");
      Serial.println(realChar);
    }
    
    if(clearing || realChar == 'c' || realChar == 'C') { // Check if we should clear the screen
      boolean cleared = false;
      clearing = true;
      if(clearing && clearData[0] == -1) { // If this is the first digit we're setting
        int tmp = inChar - 48;
        if(tmp >= 0 && tmp <= 9) clearData[0] = tmp; // Drop it into the array
        else clearData[1] = 0;
      } else if(clearing && clearData[1] == -1) { // If it's the second
        int tmp = inChar - 48;
        if(tmp >= 0 && tmp <= 9) clearData[1] = tmp; // Drop it into the array
        else clearData[1] = 0;
        
        int sizeToPush = clearData[0] * 10 + clearData[1]; // Calculate the size to shift
        if(verbose) { // Yell out that we're clearing if we are loud
          Serial.print("Clearing: (");
          Serial.print(clearData[0]);
          Serial.print(", ");
          Serial.print(clearData[1]);
          Serial.println(")");
        }
        clearSerial(sizeToPush); // Move 'em up
        
        // Reset all our data.
        clearData[0] = -1;
        clearData[1] = -1;
        clearing = false;
        cleared = true;
      }
      if(!cleared) {
        clearing = true;
        if(verbose) { // If we've cleared and we can be loud, tell the user that we've cleared
          Serial.print("Clearing: (");
          Serial.print(clearData[0]);
          Serial.print(", ");
          Serial.print(clearData[1]);
          Serial.println(")");
        }
      }
    } else { // If we aren't clearing
      if(readingIO != 'n') { // Check if we're reading IO
        int pin = inChar - 48; // Get the pin number to set
        if(pin >= 0 && pin <= 13) { // Is it in bounds?
          if(readingIO == 'i') { // Are we setting an input pin?
            pinMode(pin, INPUT);
            pinModes[pin] = INPUT;
            if(verbose) {
              Serial.print("Setting mode of pin [");
              Serial.print(pin);
              Serial.println("] to [I].");
            }
          } else if(readingIO == 'o') { // Are we setting an output pin?
            pinMode(pin, OUTPUT);
            pinModes[pin] = OUTPUT;
            if(verbose) {
              Serial.print("Setting mode of pin [");
              Serial.print(pin);
              Serial.println("] to [O].");
            }
          }
        }
        readingIO = 'n'; // Reset the readingIO
      } else if(realChar == 'i' || realChar == 'I') { // Set the readingIO data to read for an input pin
        readingIO = 'i';
        Serial.println("Waiting for next pin character to set as an input...");
      } else if(realChar == 'o' || realChar == 'O') { // Set the readingIO data to read for an input pin
        readingIO = 'o';
        Serial.println("Waiting for next pin character to set as an ouput...");
      } else if(inChar - 48 >= 0 && inChar - 48 < pinCount) { // Set the data of a pin.
        if(pinModes[inChar - 48] == OUTPUT) checkMessage(inChar - 48); // But only if it's an output pin (saving memory...)
      } else if(realChar == 'p' || realChar == 'P') { // Should we print the pins?
        printPins();
      } else if(realChar == 'h' || realChar == 'H') { // Should we show them help?
        printHelp();
      } else if(realChar == 'l' || realChar == 'L') { // Should we start capturing power?
        capturing = true;
      } else if(realChar == 'v' || realChar == 'V') { // Should we flip verbosity?
        verbose = !verbose;
        Serial.print("Turning verbosity ");
        Serial.println((verbose ? "on" : "off"));
      } else {
        Serial.println("Invalid character! Enter [h] or [H]!"); // "What did you give us?"
      }
    }
  }
}

// Called whenever we want to switch the state of a pin
void checkMessage(int pin) {
  pinState[pin] = pinState[pin] == 1 ? 0 : 1; // Change the state in the data we have
  int power = pinState[pin] == 1 ? HIGH : LOW; // Check the power we need to use
  digitalWrite(pins[pin], power); // Write the power to the pin
  Serial.print("Turning pin ["); // Now tell the user about it
  Serial.print(pins[pin]);
  Serial.print("] to state [");
  Serial.print(power);
  Serial.println("].");
}

// Called whenever the user wants to see the status of all the pins
void printPins() {
  Serial.println("Pin | State | Pin Mode"); // Tell them the key of the table
  for(int i = 0; i < pinCount; i++) { // Loop through all pins, telling the user each pin's data
    Serial.print((i < 10 ? "0" : ""));
    Serial.print(pins[i]);
    Serial.print(" (");
    Serial.print((char)(i + 48));
    Serial.print(")");
    Serial.print(" | ");
    Serial.print(pinState[i]);
    Serial.print(" | ");
    Serial.println((pinModes[i] == OUTPUT ? "OUT" : "IN"));
  }
  Serial.print("\n");
}

// Called whenever the user wants some help. Literally just talking to the user.
void printHelp() {
  Serial.println("==== HELLP ====");
  Serial.println("[p]   or [P]      | Displays current pin status");
  Serial.println("[h]   or [H]      | Shows this help pane");
  Serial.println("[ix]  or [Ix]     | Set a pin as an input pin");
  Serial.println("[ox]  or [Ox]     | Set a pin as an output pin");
  Serial.println("[v]   or [V]      | Switches verbosity off if it is currently on and vice-verser");
  Serial.println("[l]   or [L]      | Listen on all pins for any power changes - essentially power sniffing - for a full second");
  Serial.println("[cxx] or [Cxx]    | \"Clears\" the screen (push all messages up xx amount of lines)");
  Serial.println("\n   Where 'x' represents a pin character - except for the case of clear where only digits are accepted, ie 00 is min, 99 is max");
  Serial.print("\n");
}

// Called when the user wants to clear a number of lines from the Monitor.
void clearSerial(int lines) {
  for(int i = 0; i < lines; i++) {
    Serial.print("\n"); // User print("\n") instead of println("") because it's memory cheap
  }
}