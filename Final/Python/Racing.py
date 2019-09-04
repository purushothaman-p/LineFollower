import struct
import serial
from time import sleep
from pynput.keyboard import Key, Controller

h =0
def press(a):
    a = int(a)            
    if(a == 0):
        keyboard.press(Key.left)
        
    elif(a == 1):
        keyboard.press(Key.right)
        
    elif(a == 2):
        keyboard.press(Key.up)
        keyboard.press(Key.left)
    elif(a == 3):
        keyboard.press(Key.up)
        keyboard.press(Key.right)
    elif(a == 4):
        keyboard.press(Key.up)

if __name__ == '__main__':
    arduino = serial.Serial()
    keyboard = Controller()
    arduino.setDTR(1)
    sleep(2)
    arduino.setDTR(0)
    
    try:
        arduino = serial.Serial('COM10', timeout=1, baudrate=9600)
    except:
        print('Serial not available')

    while 1:
        getVal = (arduino.readline())
        if (h != getVal):
            h= getVal
            keyboard.release(Key.left)
            keyboard.release(Key.right)
            keyboard.release(Key.up)
        print(getVal)
        try:
            press(getVal)
        except ValueError:
            pass
