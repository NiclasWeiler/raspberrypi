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

# Use BCM GPIO references
# instead of physical pin numbers
GPIO.setmode(GPIO.BCM)

# input from GPIO7
GPIO.setup(7, GPIO.IN)

# Set WaitTime between alarm check to 0.5 second
WaitTimeAlarm = 0.5        # time delay in alarm loop for checking movement detector
numOfSec = 10              # record time for movie and also used for stepmotor movement
allreadyDetected = False

# Define GPIO signals to use for stepmotor
# Physical pins 11,15,16,18
# GPIO17,GPIO22,GPIO23,GPIO24
StepPins = [17,22,23,24]

# Set all pins used for stepmotor as output
for pin in StepPins:
  print "Setup pins"
  GPIO.setup(pin,GPIO.OUT)
  GPIO.output(pin, False)

  
# This function will replace all space characters in "str" with a "_"
def replaceSpace( str ):
  st = list(str)
  res = str.find(" ")
  while res != -1:
    st[res] = "_"
    res = str.find(" ")
    str = ''.join(st)
  return str


# This function will record a movie while the camera is turned
# from left to right and back right to left to the same position.
# The filename will include date and time.
def recordMovie( filename ):

  # Initialise variables
  StepCounter = 0
  WaitTime = 1.95/float(1000) # stepmotor moved every 5 ms
  # Set camera direction
  # Set to 1 or 2 for clockwise
  # Set to -1 or -2 for anti-clockwise
  StepDir = 1

  # Define advanced sequence for stepmotor
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
  numOfSeq = numOfSec*50 # one second is equal to 50 sequencies

  # Start recording
  print    ("raspivid -o " + "videos/" + fileName + " -t " + str((numOfSec +2)*1000))
  os.system("raspivid -o " + "videos/" + fileName + " -t " + str((numOfSec +2)*1000) + " &")
  time.sleep(1) # wait for 1 sec
  seqNum = 0
  print str(time.time())
  # Start moving camera
  while seqNum < numOfSeq: # run for numOfSec seconds
#    print StepCounter,
#    print Seq[StepCounter]

    for pin in range(0,4):
      xpin=StepPins[pin]# Get GPIO
      if Seq[StepCounter][pin]!=0:
        # print " Enable GPIO %i" %(xpin)
        GPIO.output(xpin, True)
      else:
        GPIO.output(xpin, False)

    StepCounter += StepDir

    # If we reach the end of the sequence
    # start again
    if (StepCounter>=StepCount):
      StepCounter = 0
      seqNum +=1
    if (StepCounter<0):
      StepCounter = StepCount+StepDir
      seqNum +=1
    if seqNum > numOfSeq/2: # change direction and move the camera
      StepDir = -1          # back to original position
      

    # Wait before moving on
    time.sleep(WaitTime)
  print str(time.time())

# Start main loop
while True:
  move_detected = not GPIO.input(7) # movement -> 0, no movement -> 1
  print move_detected
  if ((move_detected) & (not allreadyDetected)):
    allreadyDetected = True
    recordCounter +=1
    localtime = time.asctime( time.localtime(time.time()) )
    print "Movie recorded local time: " + localtime
    timestring = replaceSpace(localtime)
    fileName = 'movie_' + timestring + '.h264'
    recordMovie(fileName)
  else:
    allreadyDetected = move_detected

  # Wait before moving on
  time.sleep(WaitTimeAlarm)



