/*
 * Arduino UNO Pin Switch Program.
 * Created by Jarod Brennfleck.
 * Copyright 2015 Jarod Brennfleck
 * Do not steal credit bro. Effort was put into this.
 */

const int pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
const int pinCount = 14;
const int captureDelay = 4;
const int capturePoints = 1000 / captureDelay;

int clearData[] = {-1, -1};
boolean clearing = false;
char readingIO = 'n';
boolean verbose = true;
int pinState[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int pinModes[] = {INPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT, OUTPUT};
boolean capturing = false;
int captured[pinCount][capturePoints];
int captureIndex = 0;
int startCaptureTime = -1;

void setup() {
  for(int i = 0; i < pinCount; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
  Serial.begin(9600);
  delay(1500);
  Serial.println("\n\nControl the arduino pins [0] to [13]. All pins are set to output.");
  Serial.println("Character to pin, and pin statuses are as follows:");
  printPins();
  printHelp();
}

void loop() {
  for(int pin = 0; pin < pinCount; pin++) {
    if(pinModes[pin] == INPUT) {
      pinState[pin] = digitalRead(pin);
    }
  }
  if(capturing) {
    if(startCaptureTime <= -1) startCaptureTime = millis();
    if(millis() - startCaptureTime >= captureIndex * captureDelay) {
      for(int i = 0; i < pinCount; i++) {
        captured[captureIndex][i] = pinState[i];
      }
      captureIndex++;
      if(captureIndex >= capturePoints) {
        capturing = false;
        captureIndex = 0;
      }
    }
  }
}

void serialEvent() {
  while(Serial.available()) {
    int inChar = Serial.read();
    char realChar = (char)inChar;
    
    if(verbose){
      Serial.print("Received: ");
      Serial.print(inChar);
      Serial.print(", ");
      Serial.println(realChar);
    }
    
    if(clearing || realChar == 'c' || realChar == 'C') {
      boolean cleared = false;
      if(clearing && clearData[0] == -1) {
        int tmp = inChar - 48;
        if(tmp >= 0 && tmp <= 9) clearData[0] = tmp;
      } else if(clearing && clearData[1] == -1) {
        int tmp = inChar - 48;
        if(tmp >= 0 && tmp <= 9) clearData[1] = tmp;
        int sizeToPush = clearData[0] * 10 + clearData[1];
        if(verbose) {
          Serial.print("Clearing: (");
          Serial.print(clearData[0]);
          Serial.print(", ");
          Serial.print(clearData[1]);
          Serial.println(")");
        }
        clearSerial(sizeToPush);
        clearData[0] = -1;
        clearData[1] = -1;
        clearing = false;
        cleared = true;
      }
      if(!cleared) {
        clearing = true;
        if(verbose) {
          Serial.print("Clearing: (");
          Serial.print(clearData[0]);
          Serial.print(", ");
          Serial.print(clearData[1]);
          Serial.println(")");
        }
      }
    } else {
      if(readingIO != 'n') {
        int pin = inChar - 48;
        if(pin >= 0 && pin <= 13) {
          if(readingIO == 'i') {
            pinMode(pin, INPUT);
            pinModes[pin] = INPUT;
            if(verbose) {
              Serial.print("Setting mode of pin [");
              Serial.print(pin);
              Serial.println("] to [I].");
            }
          } else if(readingIO == 'o') {
            pinMode(pin, OUTPUT);
            pinModes[pin] = OUTPUT;
            if(verbose) {
              Serial.print("Setting mode of pin [");
              Serial.print(pin);
              Serial.println("] to [O].");
            }
          }
        }
        readingIO = 'n';
      } else if(realChar == 'i' || realChar == 'I') {
        readingIO = 'i';
        Serial.println("Waiting for next pin character to set as an input...");
      } else if(realChar == 'o' || realChar == 'O') {
        readingIO = 'o';
        Serial.println("Waiting for next pin character to set as an output...");
      } else if(inChar - 48 >= 0 && inChar - 48 < pinCount) {
        checkMessage(inChar - 48);
      } else if(realChar == 'p' || realChar == 'P') {
        printPins();
      } else if(realChar == 'h' || realChar == 'H') {
        printHelp();
      } else if(realChar == 'l' || realChar == 'L') {
        capturing = true;
      } else if(realChar == 'v' || realChar == 'V') {
        verbose = !verbose;
        Serial.print("Turning verbosity ");
        Serial.println((verbose ? "on" : "off"));
      } else {
        Serial.println("Invalid character! Enter [h] or [H]!");
      }
    }
  }
}

void checkMessage(int pin) {
  if(pinModes[pin] == INPUT) {
    Serial.print("Error: Pin [");
    Serial.print(pins[pin]);
    Serial.print("] is an input pin! Your operation is invalid!");
  } else {
    pinState[pin] = pinState[pin] == 1 ? 0 : 1;
    int power = pinState[pin] == 1 ? HIGH : LOW;
    digitalWrite(pins[pin], power);
    Serial.print("Turning pin [");
    Serial.print(pins[pin]);
    Serial.print("] to state [");
    Serial.print(power);
    Serial.println("].");
  }
}

void printPins() {
  Serial.println("Pin | State | Pin Mode");
  for(int i = 0; i < pinCount; i++) {
    Serial.print((i < 10 ? "0" : ""));
    Serial.print(pins[i]);
    Serial.print(" (");
    Serial.print((char)(i + 48));
    Serial.print(")");
    Serial.print(" | ");
    Serial.print(pinState[i]);
    Serial.print(" | ");
    Serial.println((pinModes[i] == OUTPUT ? "OUTPUT" : "INPUT"));
  }
  Serial.print("\n");
}

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
void clearSerial(int lines) {
  for(int i = 0; i < lines; i++) {
    Serial.print("\n");
  }
}