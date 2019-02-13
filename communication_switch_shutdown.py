import time
import serial
import codecs
import os
import RPi.GPIO as GPIO

ser = serial.Serial("/dev/ttyUSB0",19200)
GPIO.setmode(GPIO.BCM)
pin = 4
GPIO.setup(pin,GPIO.IN,GPIO.PUD_UP)
GPIO.add_event_detect(pin,GPIO.FALLING,callback = callBackTest,bouncetime = 300)

def callBackTest(channel):
    print("callback")

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
