boolean verbose = true;

void setup() {
	Serial.begin(9600);
	Serial.println("Arduino and Python communications!");
	Serial.println("Communicate to your arduino using PySerial!");
}

void loop() {
	delay(1000);
}
void serialEvent() {
	while(Serial.available()) {
		int inChar = Serial.read();
		char realChar = (char)inChar;
		
		printOut("Char: " + realChar, verbose);
	}
}

void printOut(Object message, boolean verbose) {
	if(verbose) Serial.print("v: ");
	Serial.println(message);
}