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
#import os
import subprocess
import time
import RPi.GPIO as GPIO


# Use BCM GPIO references
# instead of physical pin numbers
GPIO.setmode(GPIO.BCM)

# input from GPIO7
GPIO.setup(7, GPIO.IN)

# Set WaitTime to 0.5 second
WaitTime = 0.5
allreadyDetected = False
counter = 1

filename = "temperature_message.txt"

# Start main loop
while True:
  move_detected = not GPIO.input(7) # movement -> 0, no movement -> 1
  print move_detected
  if ((move_detected) & (not allreadyDetected)):

    # get local time
    localtime = time.asctime( time.localtime(time.time()) )

    # Write info into file
    messageFile = open(filename, 'a')
    messageFile.write(localtime + " " + "Movement detected\n")
    messageFile.close()

    allreadyDetected = True
    fileName = 'movie_' + str(counter) + '.h264'
    print ("raspivid -o " + fileName + " -t 5000")
    subprocess.call(["raspivid", "-o", fileName, "-t", "5000"])
    counter +=1
  else:
    allreadyDetected = move_detected

  # Wait before moving on
  time.sleep(WaitTime)
