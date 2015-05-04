import serial
import get_com_ports

print("Here's a list of connected COM ports...")
"\n".join(get_com_ports.serial_ports())
com_port = input("What is your Arduino connected to? (eg 'COM4') ")

arduino = serial.Serial(com_port, 9600, timeout=0.1)

while True:
	data = arduino.readline()[:-2] #the last bit gets rid of the new-line chars
	if data: print(data, end="")

