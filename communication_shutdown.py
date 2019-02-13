import time
import serial
import codecs
import os


ser = serial.Serial("/dev/ttyUSB0",19200)
def shutdown():
    print("shutdown by command...")
    time.sleep(1)
    #We should clean gpio(maybe...)
    os.system("sudo shutdown -h now")

def im920_receive():
    if(ser.inWaiting()):
        stock = (codecs.decode(ser.readline()).split(',')[2]).split(':')[1]
        print(stock)
        if(stock == 'shutdown\r\n'):
          shutdown()
while(1):
    im920_receive()
