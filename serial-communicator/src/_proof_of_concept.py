import serial
import time

ARDUINO_TTY = "/dev/ttyACM0"

s = serial.Serial(ARDUINO_TTY, 115200, timeout = 1)

s.reset_input_buffer()
s.write("hello".encode('utf-8'))
while True:
    time.sleep(0.01)
    if s.in_waiting > 0: 
        response = s.readline().decode('utf-8').rstrip()
        print(response)
        break