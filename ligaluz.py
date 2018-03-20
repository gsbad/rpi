import RPi.GPIO as GPIO
import sys

def inicializaBoard():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setwarnings(False)

def definePinoComoSaida(numeroPino):
    GPIO.setup(numeroPino, GPIO.OUT)
 
def escreveParaPorta(numeroPino,estadoPorta):
    GPIO.output(numeroPino, estadoPorta)

numeroPino = int(sys.argv[1])
estadoPorta = int(sys.argv[2])

inicializaBoard()
definePinoComoSaida(numeroPino)
escreveParaPorta(numeroPino, estadoPorta)
