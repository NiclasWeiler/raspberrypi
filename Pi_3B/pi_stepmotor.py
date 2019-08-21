#-------------------------------------------------------------------------------
# Name:        module2
# Purpose:
#
# Author:      Niclas
#
# Created:     20-03-2018
# Copyright:   (c) Niclas 2018
# Licence:     <your licence>
#-------------------------------------------------------------------------------

#!/usr/bin/python
# Import required libraries
import sys
import time
import RPi.GPIO as GPIO


def moveCamera(distance, direction):

  # Use BCM GPIO references
  # instead of physical pin numbers
  GPIO.setmode(GPIO.BCM)

  # Define GPIO signals to use
  # Physical pins 11,15,16,18
  # GPIO21,GPIO22,GPIO23,GPIO24
  StepPins = [21,22,23,24]
  WaitTime = 15/float(1000)

  # Set all pins as output
  for pin in StepPins:
    print ("Setup pins")
    GPIO.setup(pin,GPIO.OUT)
    GPIO.output(pin, False)

  # Define advanced sequence
  # as shown in manufacturers datasheet
  Seq = [[1,0,0,1],
         [1,0,0,0],
         [1,1,0,0],
         [0,1,0,0],
         [0,1,1,0],
         [0,0,1,0],
         [0,0,1,1],
         [0,0,0,1]]

  StepCount = len(Seq)

  # Set to 1 or 2 for clockwise
  # Set to -1 or -2 for anti-clockwise
  if direction == "left":
    StepDir = -1
  else:
    StepDir = 1

  # Initialise variables
  StepCounter = 0
  distanceCount = 0

  # Start main loop
  while distanceCount < distance:

  #  print (StepCounter)
  #  print (Seq[StepCounter])

    for pin in range(0,4):
      xpin=StepPins[pin]# Get GPIO
      if Seq[StepCounter][pin]!=0:
        #print (" Enable GPIO %i" %(xpin))
        GPIO.output(xpin, True)
      else:
        GPIO.output(xpin, False)

    StepCounter += StepDir
    distanceCount += 1

    # If we reach the end of the sequence
    # start again
    if (StepCounter>=StepCount):
      StepCounter = 0
    if (StepCounter<0):
      StepCounter = StepCount+StepDir

    # Wait before moving on
    time.sleep(WaitTime)

  GPIO.cleanup()

# Main

#distance = 100    # Number of sequences (default)
# Read distance from command line, if available
#if len(sys.argv)>2:
  #distance = int(sys.argv[2])

#direction = "right"    # rotation direction (default)
#if len(sys.argv)>3:
    #direction = sys.argv[3]

#moveCamera(distance, direction)
