import time
import serial
import codecs
import os
import RPi.GPIO as GPIO

ser = serial.Serial("/dev/ttyUSB0",19200)
GPIO.setmode(GPIO.BCM)
pin = 4 
GPIO.setup(pin,GPIO.IN,GPIO.PUD_UP)

#CLEAN BUFFER FOR SECURE
def clean_buf():
  while(ser.inWaiting()):
    wastebox = ser.readline()

#RECEIVE
def im920_receive():
    if(ser.inWaiting()):
        stock = (codecs.decode(ser.readline()).split(',')[2]).split(':')[1]
        print(stock)
        if(stock == 'shutdown\r\n'):
          shutdown()

#"TXDT" AND "\r\n" ARE IM920'S COMMANDS
def im920_send(command):
    ser.write(str.encode("TXDT"+str(command)+"\r\n"))
clean_buf()

def shutdown():
    print("shutdown by command...")
    time.sleep(1)
    os.system("sudo shutdown -h now")

def callBackTest(channel):
    shutdown()

GPIO.add_event_detect(pin,GPIO.FALLING,callback = callBackTest,bouncetime = 300)

SRCLK = 36
RCLK  = 38
SER   = 40

def pin_init():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(SRCLK, GPIO.OUT)
    GPIO.setup(RCLK, GPIO.OUT)
    GPIO.setup(SER, GPIO.OUT)
    GPIO.output(SRCLK,0)
    GPIO.output(RCLK,0)
    GPIO.output(SER,0)

def pin_clear():
    GPIO.output(SER,0)
    for j in range(8):
        GPIO.output(SRCLK,0)
        GPIO.output(SRCLK,1)
        GPIO.output(RCLK,0)
        GPIO.output(RCLK,1)

def pin_expand(i):
    GPIO.output(SER,1)
    for j in range(i):
        GPIO.output(SRCLK,0)
        GPIO.output(SRCLK,1)
        GPIO.output(SER,0)
    GPIO.output(RCLK,0)
    GPIO.output(RCLK,1)

#main
#pin_init()
#pin_expand(5)

