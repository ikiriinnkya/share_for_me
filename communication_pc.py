import time
import serial
import codecs

#ENTER YOUR FAVORITE PORT AND RATE
ser = serial.Serial("/dev/ttyUSB0",19200)

#CLEAN BUFFER FOR SECURE
def clean_buf():
  while(ser.inWaiting()):
    wastebox = ser.readline()

#CHANGING FORMAT
def im920_receive():
  if(ser.inWaiting()):
    print((codecs.decode(ser.readline()).split(',')[2]).split(':')[1])

#"TXDT" AND "\r\n" ARE IM920'S COMMANDS
def im920_send(command):
  ser.write(codecs.encode("TXDT"+str(command)+"\r\n"))

clean_buf()
while(1):
  h = raw_input()
  im920_send(h)


