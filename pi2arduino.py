#!/usr/bin/env python

import serial
import time

path = 'bridge.txt'
ser = serial.Serial('/dev/ttyUSB0',9600)

while True:
	time.sleep(0.01)
	with open(path) as f:
		for i in range(4):
			s_line = f.readline()
			if(i == 0):
				##print("0" + s_line)
				if(s_line == "MOVE_FORWARD\n"):
					print("go!!!")
					ser.write(b"G")
				elif(s_line == "MOVE_BACK\n"):
					print("back!")
					ser.write(b"B")
				elif(s_line == "MOVE_RIGHT\n"):
					print("right")
					ser.write(b"R")
				elif(s_line == "MOVE_LEFT\n"):
					print("left")
					ser.write(b"L")
				else:
					ser.write(b"S")
				
			elif(i == 1):
				##print("1" + s_line)
				if(s_line == "ROTATE_RIGHT\n"):
					print("rotate_r")
					ser.write(b"r")
				elif(s_line == "ROTATE_LEFT\n"):
					print("rotate_l")
					ser.write(b"l")
				#else:
					#ser.write(b"S")

			elif(i == 2):
				##print("2" + s_line)
				if(s_line == "PASS\n"):
					print("pass")
					ser.write(b"P")
				elif(s_line == "PICKUP\n"):
					print("pick")
					ser.write(b"U");
				#else:
					#ser.write(b"S")
			elif(i == 3):
				##print("3" + s_line)
				if(s_line == "KICK\n"):
					print("K")
				#else:
					#ser.write(b"S")
