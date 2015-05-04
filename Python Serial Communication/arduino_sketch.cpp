void setup() {
	Serial.begin(9600);
	Serial.println("Testing, testing... 1... 2... 3...");
}
void loop() {
	Serial.println("Broadcasting [LIVE] from the Arduino!");
	delay(1000);
}